#include <device/ata/sata.h>
#include <panic.h>
#include <x86/ports.h>
#include <memory/page_frame_allocator.h>
#include <string.h>
#include <stdbool.h>

#define MAX_DEVICES 32

#define HBA_PI_ACTIVE (1 << 0)
#define HBA_PI_PRESENT (3 << 8)
 
#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

typedef struct hba_cmd_header
{
    // DW0
    uint8_t cfl : 5;    // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a : 1;      // ATAPI
    uint8_t w : 1;      // Write, 1: H2D, 0: D2H
    uint8_t p : 1;      // Prefetchable
 
    uint8_t r : 1;      // Reset
    uint8_t b : 1;      // BIST
    uint8_t c : 1;      // Clear busy upon R_OK
    uint8_t reserved0 : 1; // Reserved
    uint8_t pmp : 4;    // Port multiplier port
 
    uint16_t prdtl;     // Physical region descriptor table length in entries
 
    // DW1
    volatile int32_t prdbc; // Physical region descriptor byte count transferred
 
    // DW2, 3
    uint32_t ctba;      // Command table descriptor base address
    uint32_t ctbau;     // Command table descriptor base address upper 32 bits
 
    // DW4 - 7
    uint32_t reserved1[4]; // Reserved
} __attribute__ ((packed)) hba_cmd_header_t;

typedef struct hba_port
{
    uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
    uint32_t clbu;      // 0x04, command list base address upper 32 bits
    uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
    uint32_t is;        // 0x10, interrupt status
    uint32_t ie;        // 0x14, interrupt enable
    uint32_t cmd;       // 0x18, command and status
    uint32_t reserved0; // 0x1C, Reserved
    uint32_t tfd;       // 0x20, task file data
    uint32_t sig;       // 0x24, signature
    uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;      // 0x30, SATA error (SCR1:SError)
    uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
    uint32_t ci;        // 0x38, command issue
    uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;       // 0x40, FIS-based switch control
    uint32_t reserved[11];// 0x44 ~ 0x6F, Reserved
    uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} __attribute__ ((packed)) hba_port_t;

typedef struct hba_mem
{
    uint32_t cap;       // Host capability
    uint32_t ghc;       // 0x04, Global host control
    uint32_t is;        // 0x08, Interrupt status
    uint32_t pi;        // 0x0C, Port implemented
    uint32_t vs;        // 0x10, Version
    uint32_t ccc_ctl;   // 0x14, Command completion coalescing control
    uint32_t ccc_pts;   // 0x18, Command completion coalescing ports
    uint32_t em_loc;    // 0x1C, Enclosure management location
    uint32_t em_ctl;    // 0x20, Enclosure management control
    uint32_t cap2;      // 0x24, Host capabilities extended
    uint32_t bohc;      // 0x28, BIOS/OS handoff control and status
 
    uint8_t reserved[0x74];
    uint8_t vendor[0x60];
 
    hba_port_t ports[];    // 1 ~ 32
} __attribute__ ((packed)) hba_mem_t;

typedef struct hba_device
{
    hba_port_t* port;
    hba_cmd_header_t* cmd_header;
    uint8_t* fis;
} hba_device_t;

static hba_device_t devices[MAX_DEVICES];
static size_t registered_devices;

static void stop_cmd(hba_port_t* port)
{
    // Clear ST (bit0) and FRE (bit4)
    port->cmd &= ~HBA_PxCMD_ST;
    port->cmd &= ~HBA_PxCMD_FRE;
 
    // Wait until FR (bit14), CR (bit15) are cleared
    while((port->cmd & HBA_PxCMD_FR) || (port->cmd & HBA_PxCMD_CR));
}

void start_cmd(hba_port_t* port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);
 
    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
}

static void configure_device(hba_device_t* device)
{
    uint64_t page0 = (uint64_t)pfa_alloc_page();
    uint64_t page1 = (uint64_t)pfa_alloc_page();
    uint64_t page2 = (uint64_t)pfa_alloc_page();

    uint64_t cmd_address = page0;
    uint32_t cmd_low = cmd_address & 0xFFFFFFFFLL;
    uint32_t cmd_high = (cmd_address >> 32) & 0xFFFFFFFFLL;
    
    uint64_t fis_address = page0 + 1024;
    uint32_t fis_low = fis_address & 0xFFFFFFFFLL;
    uint32_t fis_high = (fis_address >> 32) & 0xFFFFFFFFLL;

    stop_cmd(device->port); // Stop command engine

    device->port->clb = cmd_low;
    device->port->clbu = cmd_high;
    memset((void*)cmd_address, 0, 1024);

    device->port->fb = fis_low;
    device->port->fbu = fis_high;
    memset((void*)fis_address, 0, 256);

    hba_cmd_header_t* cmdheader = (hba_cmd_header_t*)cmd_address;
    for (int i = 0; i < 32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table

        uint64_t cmd_addr = i < 16 ? page1 : page2;
        cmd_addr += 256 * (i % 16);

        uint32_t cmd_addr_low = cmd_addr & 0xFFFFFFFFLL;
        uint32_t cmd_addr_high = (cmd_addr >> 32) & 0xFFFFFFFFLL;

        cmdheader[i].ctba = cmd_addr_low;
        cmdheader[i].ctbau = cmd_addr_high;
        memset((void*)cmd_addr, 0, 256);
    }

    start_cmd(device->port);    // Start command engine
}

void sata_init()
{
    registered_devices = 0;
}

size_t sata_devices()
{
    return registered_devices;
}

void sata_register_pci_device(pci_device_t* device)
{
    uint32_t starting_device = registered_devices;

    hba_mem_t* hba_mem = (void*)device->base5;
    
    for(int bit = 0; bit < 32; bit++)
        if(hba_mem->pi & (1 << bit)) // bit is set: device exists
        {
            hba_port_t* port = &(hba_mem->ports[bit]);
            if((port->ssts & HBA_PI_ACTIVE) && (port->ssts & HBA_PI_PRESENT))
            {
                if(registered_devices >= MAX_DEVICES)
                    kenrel_panic("Only %d SATA devices are supported.", MAX_DEVICES);

                hba_device_t hba_device;
                hba_device.port = port;

                devices[registered_devices++] = hba_device;
            }
        }
    
    if(starting_device != registered_devices)
    {
        uint64_t device_size = sizeof(hba_mem_t) + (registered_devices - starting_device) * sizeof(hba_port_t);
        pfa_deinit_region(device->base5, device_size);

        for(int i = starting_device; i < registered_devices; i++)
            configure_device(devices[i].port);
    }
}

void sata_read(size_t device, uint64_t lba, uint8_t count, void* address)
{

}

void sata_write(size_t device, uint64_t lba, uint8_t count, void* address)
{

}
