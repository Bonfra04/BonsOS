#include <drivers/usb/uhci/uhci.h>
#include <drivers/usb/usb.h>
#include <memory/pfa.h>
#include <memory/paging.h>
#include <timers/pit.h>
#include <panic.h>
#include <io/ports.h>

#include <containers/darray.h>
#include <linker.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

#include "uchi_regs.h"

#include <log.h>

#define port_set(addr, data) outportw(addr, inportw(addr) | (data));
#define port_clear(addr, data) outportw(addr, inportw(addr) & ~(data));

typedef struct uhci_controller
{
    const pci_dev_info_t* pci;
    uint64_t num_ports;
    uint32_t io;
    volatile uint32_t* frame_list;
} uhci_controller_t;

typedef struct transfer_descriptor
{
    uint32_t link;
    uint32_t flags;
    uint32_t maxlen;
    uint32_t bufptr;
    uint32_t reserved[4];
} __attribute__((packed)) transfer_descriptor_t;

typedef struct queue_head
{
    uint32_t head_link;
    uint32_t element_link;
    uint32_t reserved[2];
} __attribute__((packed)) queue_head_t;

static bool controller_reset(uhci_controller_t* controller)
{
    kernel_trace("Resetting UHCI controller");

    // issue global reset
    for(int i = 0; i < 5; i++)
    {
        outportw(controller->io + IO_USBCMD, USBCMD_GRESET);
        pit_sleep(11);
        outportw(controller->io + IO_USBCMD, 0);
    }

    // check default values
    if(inportw(controller->io + IO_USBCMD) != 0)
        return false;
    if(inportw(controller->io + IO_USBSTS) != USBSTS_HALTED)
        return false;

    // clear status register (write/clear)
    outportw(controller->io + IO_USBSTS, 0xFF);

    // check default values
    if(inportb(controller->io + IO_SOFMOD) != SOFMOD_64)
        return false;

    // issue controller reset
    outportw(controller->io + IO_USBCMD, USBCMD_HCRESET);
    pit_sleep(42);
    if(inportw(controller->io + IO_USBCMD) & USBCMD_HCRESET)
        return false;

    kernel_trace("Reset successful");

    return true;
}

static void* alloc_frame_list()
{
    uint32_t* frame_list = (uint32_t*)pfa_alloc(1);
    if(frame_list == NULL)
        return NULL;
    for(uint32_t i = 0; i < 1024; ++i)
        frame_list[i] = FRAMELIST_TERMINATE;
    return frame_list;
}

static bool port_present(uhci_controller_t* controller, uint64_t port)
{
    uint16_t addr = controller->io + IO_PORTSC + port * 2;

    // bit 7 should always be 1 if the port exists
    if((inportw(addr) & (1 << 7)) == 0)
        return false;

    // try to clear it
    port_clear(addr, 1 << 7);
    if((inportw(addr) & (1 << 7)) == 0)
        return false;

    // try to write-clear it
    port_set(addr, 1 << 7);
    if((inportw(addr) & (1 << 7)) == 0)
        return false;

    // try to set 1:3, they sould come back as zero
    port_set(addr, PORTSC_ENABLE_CHANGE | PORTSC_STATUS_CHANGE);
    if((inportw(addr) & (PORTSC_ENABLE_CHANGE | PORTSC_STATUS_CHANGE)) != 0)
        return false;

    return true;
}

static bool init_controller(uhci_controller_t* controller)
{
    if(!controller_reset(controller))
        return false;

    // disable interrupts
    outportw(controller->io + IO_USBINTR, 0);
    // set frame number to 0
    outportw(controller->io + IO_FRNUM, 0);

    // set frame list base address
    controller->frame_list = alloc_frame_list();
    if(controller->frame_list == NULL)
        return false;
    outportd(controller->io + IO_FRBASEADD, (uint32_t)(uint64_t)controller->frame_list);

    // set sof mod
    outportb(controller->io + IO_SOFMOD, SOFMOD_64);

    // clear status register (write/clear)
    outportw(controller->io + IO_USBSTS, 0xFF);

    // count ports
    controller->num_ports = 0;
    while(port_present(controller, controller->num_ports))
        controller->num_ports++;

    // end of initial config and start controller
    outportw(controller->io + IO_USBCMD, USBCMD_CF | USBCMD_RUNSTOP);

    return true;
}

static bool reset_port(void* data, uint64_t port)
{
    uhci_controller_t* controller = data;
    if(port >= controller->num_ports)
        return false;
    uint16_t addr = controller->io + IO_PORTSC + port * 2;

    kernel_trace("Resetting port %d", port);

    // issue port reset
    port_set(addr, PORTSC_PORT_RESET);
    pit_sleep(50);
    port_clear(addr, PORTSC_PORT_RESET);
    // wait for recovery time
    pit_sleep(10);
    
    // wait for port to enable
    for(uint8_t i = 0; i < 16; i++)
    {
        pit_sleep(10);

        uint16_t status = inportw(addr);

        // check if something is attached
        if((status & PORTSC_CONNECT_STATUS) == 0)
            break;

        // check if something changed
        if((status & (PORTSC_ENABLE_CHANGE | PORTSC_STATUS_CHANGE)) != 0)
        {
            port_set(addr, PORTSC_ENABLE_CHANGE | PORTSC_STATUS_CHANGE);
            continue;
        }

        // check if port is enabled
        if((status & PORTSC_ENABLE) != 0)
            return true;

        // enable port
        port_set(addr, PORTSC_ENABLE);
    }
   
    return false;
}

static usb_port_status_t port_status(void* data, uint64_t port)
{
    uhci_controller_t* controller = data;
    if(port >= controller->num_ports)
        return USB_PORT_STATUS_NOT_CONNECT;
    uint16_t addr = controller->io + IO_PORTSC + port * 2;

    uint16_t status = inportw(addr);

    if(status & PORTSC_CONNECT_STATUS)
        if(status & PORTSC_LOW_SPEED)
            return USB_PORT_STATUS_CONNECT_LOW_SPEED;
        else
            return USB_PORT_STATUS_CONNECT_FULL_SPEED;
    else
        return USB_PORT_STATUS_NOT_CONNECT;
}

static void sched_stop(uhci_controller_t* controller)
{
    outportw(controller->io + IO_USBCMD, inportw(controller->io + IO_USBCMD) & ~USBCMD_RUNSTOP);
    outportw(controller->io + IO_USBSTS, USBSTS_INT);
}

static void sched_run(uhci_controller_t* controller)
{
    sched_stop(controller);
    outportw(controller->io + IO_FRNUM, 0);
    outportw(controller->io + IO_USBCMD, inportw(controller->io + IO_USBCMD) | USBCMD_RUNSTOP);
}

static usb_transfer_status_t transfer_packets(void* data, uint64_t addr, uint64_t endpoint, const usb_packet_t* packets, size_t num_packets)
{
    uhci_controller_t* controller = data;

    outportw(controller->io + IO_USBSTS, USBSTS_INT);
    for(uint32_t i = 0; i < 1024; ++i)
        controller->frame_list[i] = FRAMELIST_TERMINATE;

    // volatile alignas(0x10) transfer_descriptor_t tds[num_packets];
    volatile transfer_descriptor_t* tds = pfa_alloc(1);
    memset((void*)tds, 0, sizeof(transfer_descriptor_t) * num_packets);
    for(size_t i = 0; i < num_packets; i++)
    {
        tds[i].link = i == num_packets - 1 ? TD_TERMINATE : ((uint32_t)(uint64_t)&tds[i + 1] | TD_DEPTH_FIRST);
        tds[i].flags = TD_STATUS_ACTIVE | TD_C_ERR;

        if(i == num_packets - 1)
            tds[i].flags |= TD_IOC;

        if(port_status(controller, addr) == USB_PORT_STATUS_CONNECT_LOW_SPEED)
            tds[i].flags |= TD_LOW_SPEED;
        
        tds[i].maxlen = ((packets[i].maxlen ? packets[i].maxlen - 1 : 0) << 21) | (endpoint << 15) | (addr << 8);
        
        switch (packets[i].type)
        {
        case USB_PACKET_TYPE_SETUP: tds[i].maxlen |= TD_PID_SETUP; break;
        case USB_PACKET_TYPE_IN: tds[i].maxlen |= TD_PID_IN; break;
        case USB_PACKET_TYPE_OUT: tds[i].maxlen |= TD_PID_OUT; break;
        }

        if(packets[i].toggle)
            tds[i].maxlen |= TD_DATA_TOGGLE;

        tds[i].bufptr = (uint32_t)(uint64_t)packets[i].buffer;
    }

    // log
    kernel_log("TDs:\n");
    for(int i = 0; i < num_packets; i++)
        kernel_log("[%d](0x%p): link: %#x, flags: %#x, maxlen: %#x, bufptr: %#x\n", i, &tds[i], tds[i].link, tds[i].flags, tds[i].maxlen, tds[i].bufptr);
    kernel_log("\n");

    volatile queue_head_t* qh = pfa_alloc(1);
    qh->head_link = QH_TERMINATE;
    qh->element_link = (uint32_t)(uint64_t)&tds[0];

    // for(uint32_t i = 0; i < 1024; ++i)
    //     controller->frame_list[i] = FRAMELIST_TERMINATE;
    // controller->frame_list[0] = (uint32_t)(uint64_t)qh | FRAMELIST_QH;
    outportw(controller->io + IO_USBSTS, USBSTS_INT);
    for(uint32_t i = 0; i < 1024; ++i)
        controller->frame_list[i] = (uint32_t)(uint64_t)qh | FRAMELIST_QH;

    // sched_run(controller);

    uint64_t spin = 0;
    while(!(inportw(controller->io + IO_USBSTS) & USBSTS_INT))
    {
        asm("pause");
        if(spin++ == 10000)
        {
            kernel_log("TDs stuck:\n");
            for(int i = 0; i < num_packets; i++)
                kernel_log("[%d](0x%p): link: %#x, flags: %#x, maxlen: %#x, bufptr: %#x\n", i, &tds[i], tds[i].link, tds[i].flags, tds[i].maxlen, tds[i].bufptr);
            kernel_log("\n");
        }
    }

    // sched_stop(controller);

    kernel_log("TDs after:\n");
    for(int i = 0; i < num_packets; i++)
        kernel_log("[%d](0x%p): link: %#x, flags: %#x, maxlen: %#x, bufptr: %#x\n", i, &tds[i], tds[i].link, tds[i].flags, tds[i].maxlen, tds[i].bufptr);
    kernel_log("\n");

    return USB_TRANSFER_STATUS_OK;
} 

static usb_hci_driver_t uhci_usb_driver = {
    .reset_port = reset_port,
    .port_status = port_status,
    .transfer_packets = transfer_packets
};

void uchi_register_pci(const pci_dev_info_t* pci_device)
{
    kernel_trace("Found UHCI controller (VENDOR: %X)", pci_device->dev.vendor_id);
    kernel_trace("%hhX %hhX %hhX", pci_device->bus, pci_device->device, pci_device->function);
    
    uhci_controller_t* controller = malloc(sizeof(uhci_controller_t));
    controller->pci = pci_device;

    if((pci_device->dev.base4 & PCI_BAR_IO) == 0)
        kernel_panic("UHCI controller is not port based");

    pci_set_privileges(pci_device, PCI_PRIV_MMIO | PCI_PRIV_DMA | PCI_PRIV_PIO);
    pci_write_word(controller->pci, UHCI_PCI_LEGSUP, 0x8F00);
    controller->io = pci_device->dev.base4 & ~PCI_BAR_IO;

    if(!init_controller(controller))
        return;

    usb_register_hci(controller, controller->num_ports, &uhci_usb_driver);
}

static pci_driver_t uhci_pci_driver = {
    .match = PCI_DRIVER_MATCH_CLASS | PCI_DRIVER_MATCH_SUBCLASS | PCI_DRIVER_MATCH_PROGIF,
    .class = PCI_CLASS_SBC,
    .subclass = PCI_SUBCLASS_USB,
    .progif = USB_PROG_UHCI,
    .register_device = uchi_register_pci
};

void uchi_init()
{
    pci_register_driver(&uhci_pci_driver);
}
