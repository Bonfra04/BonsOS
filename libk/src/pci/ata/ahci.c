#include <pci/ata/ahci.h>
#include <pci/ata/ata.h>
#include <memory/paging.h>
#include <memory/pfa.h>
#include <timers/pit.h>
#include <log.h>
#include <panic.h>
#include <alignment.h>

#include <linker.h>
#include <containers/darray.h>

#include <string.h>

#include "ahci_types.h"

static ahci_device_t* ahci_devices;

void ahci_init()
{
    ahci_devices = darray(ahci_device_t, 0);
}

static uint8_t get_free_cmd_slot(ahci_device_t* dev)
{
    for(size_t i = 0; i < dev->ncmd; i++)
    {
        if((dev->allocated_cmd & (1 << i)) == 0)
        {
            dev->allocated_cmd |= (1 << i);
            return i;
        }
    }

    return 0xFF;
}

static ahci_command_t allocate_command(ahci_device_t* dev)
{
    uint8_t i = get_free_cmd_slot(dev);
    if(i == 0xFF)
        return (ahci_command_t){ .cmd_table = NULL, .index = ~0 };

    volatile hba_cmd_header_t* header = &dev->cmd_header[i];
    hba_cmd_table_t* table = (void*)(header->ctba | ((uint64_t)header->ctbau << 32));

    return (ahci_command_t){ .cmd_table = table, .index = i };
}

static void free_command(ahci_device_t* dev, const ahci_command_t* cmd)
{
    dev->allocated_cmd &= ~(1 << cmd->index);
}

static uint32_t calc_bytecount(size_t count)
{
    return (((count + 1) & ~1) - 1) & 0x3FFFFF;
}

static void wait_ready(ahci_device_t* dev)
{
    while((dev->port->tfd & HBA_PxTFD_STS_BSY) || (dev->port->tfd & HBA_PxTFD_STS_DRQ))
        asm("pause");
}

static void idle_port(volatile hba_port_t* port)
{
    port->cmd &= ~HBA_PxCMD_ST;
    while(port->cmd & HBA_PxCMD_CR)
        asm("pause");

    port->cmd &= ~HBA_PxCMD_FRE;
    while(port->cmd & HBA_PxCMD_FR)
        asm("pause");
}

static bool init_port(volatile hba_port_t* port)
{ 
    uint8_t det = port->ssts & 0xF;
    uint8_t ipm = (port->ssts >> 8) & 0xFF;
    if(det != 3)
    {
        if(det == 0)
            return false; // no Device and no comms
        else if(det == 1)
            kernel_panic("Device but no comms, TODO implement COMRESET");
        else if(det == 4) // port offline
            return false;
        else
            kernel_panic("Unknown PxSSTS.det value %lu", (uint32_t)det);
    }
    if(ipm != 1)
        kernel_panic("TODO Get device out of sleep state");

    uint32_t sig = port->sig;
    if(sig == 0)
        return false;

    if(sig != SATA_SIG_ATA && sig != SATA_SIG_ATAPI)
        kernel_panic("ahci: Unknown PxSIG (%#X)", sig);

    idle_port(port);

    uint64_t page0 = (uint64_t)pfa_calloc(1);

    uint64_t cmd_address = page0;
    uint32_t cmd_low = cmd_address & 0xFFFFFFFFLL;
    uint32_t cmd_high = (cmd_address >> 32) & 0xFFFFFFFFLL;

    uint64_t fis_address = page0 + 1024;
    uint32_t fis_low = fis_address & 0xFFFFFFFFLL;
    uint32_t fis_high = (fis_address >> 32) & 0xFFFFFFFFLL;

    port->clb = cmd_low;
    port->clbu = cmd_high;

    port->fb = fis_low;
    port->fbu = fis_high;

    port->cmd |= HBA_PxCMD_ST | HBA_PxCMD_FRE;

    port->ie = 0;
    port->is = ~0u;

    pit_prepare_one_shot(1);
    pit_perform_one_shot();

    if(!(port->cmd & HBA_PxCMD_CR) || !(port->cmd & HBA_PxCMD_FR))
    {
        kernel_warn("Failed to start CMD engine");
        return false;
    }

    volatile hba_cmd_header_t* cmd_header = ptr(cmd_address);
    uint64_t page1 = (uint64_t)pfa_calloc(1);
    uint64_t page2 = (uint64_t)pfa_calloc(1);

    for (uint8_t i = 0; i < 32; i++)
    {
        uint64_t cmd_addr = i < 16 ? page1 : page2;
        cmd_addr += 256 * (i % 16);

        uint32_t cmd_addr_low = cmd_addr & 0xFFFFFFFFull;
        uint32_t cmd_addr_high = (cmd_addr >> 32) & 0xFFFFFFFFull;

        cmd_header[i].ctba = cmd_addr_low;
        cmd_header[i].ctbau = cmd_addr_high;
    }

    return true;
}

static void init_device(volatile hba_mem_t* hba, void (*registrant)(ata_device_t*))
{
    hba->ghc |= HBA_GHC_AE;
    hba->is = ~0u;

    for(uint8_t bit = 0; bit < 32; bit++)
        if(hba->pi & (1 << bit)) // bit is set device exists
        {
            volatile hba_port_t* port = &hba->ports[bit];
            paging_map(NULL, (void*)port, (void*)port, sizeof(hba_port_t), PAGE_PRIVILEGE_KERNEL);
            pfa_deinit_region((void*)port, sizeof(hba_port_t));

            if(!init_port(port))
                continue;

            ahci_device_t ahci_device;
            ahci_device.port = port;
            ahci_device.allocated_cmd = 0;
            ahci_device.ncmd = ((hba->cap >> 8) & HBA_CAP_NCS) + 1;
            ahci_device.cmd_header = (void*)(port->clb | ((uint64_t)port->clbu << 32));

            darray_append(ahci_devices, ahci_device);

            ata_device_t ata_device;
            ata_device.internal_id = darray_length(ahci_devices) - 1;
            ata_device.send_ata_cmd = ahci_send_ata_cmd;
            registrant(&ata_device);
        }
}

void ahci_register_device(pci_device_t* pci_device, void (*registrant)(ata_device_t*))
{
    pci_set_privileges(pci_device, PCI_PRIV_DMA | PCI_PRIV_MMIO);

    paging_map(NULL, ptr(pci_device->base5), ptr(pci_device->base5), sizeof(hba_mem_t), PAGE_PRIVILEGE_KERNEL);
    pfa_deinit_region(ptr(pci_device->base5), sizeof(hba_mem_t));

    init_device((volatile void*)ptr(pci_device->base5), registrant);
}

size_t ahci_ndevices()
{
    return darray_length(ahci_devices);
}

#include <timers/time.h>

bool ahci_send_ata_cmd(uint64_t dev_id, ata_command_t* command, uint8_t* data, size_t transfer_len)
{
    if(dev_id >= ahci_ndevices())
        return false;
    ahci_device_t* dev = &ahci_devices[dev_id];

    size_t n_prdts = (transfer_len + 0x3FFFFF - 1) / 0x3FFFFF;
    ahci_command_t slot = allocate_command(dev);

    if(slot.index == 0xFF)
        return false;

    dev->cmd_header[slot.index].prdtl = n_prdts;
    dev->cmd_header[slot.index].write = command->write;
    dev->cmd_header[slot.index].cfl = 5; // h2d register is 5 dwords

    volatile fis_reg_h2d_t* fis = (void*)slot.cmd_table->cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;

    fis->command = command->command;
    fis->control = 0x8;
    fis->device = 0xA0 | (1 << 6) | ((command->lba28) ? ((command->lba >> 24) & 0xF) : (0));
    fis->countl = command->n_sectors & 0xFF;
    fis->counth = (command->n_sectors >> 8) & 0xFF;

    fis->lba0 = command->lba & 0xFF;
    fis->lba1 = (command->lba >> 8) & 0xFF;
    fis->lba2 = (command->lba >> 16) & 0xFF;
    fis->lba3 = (command->lba >> 24) & 0xFF;
    fis->lba4 = (command->lba >> 32) & 0xFF;
    fis->lba5 = (command->lba >> 40) & 0xFF;

    fis->featurel = command->features & 0xFF;
    fis->featureh = (command->features >> 8) & 0xFF;

    for(size_t i = 0; i < n_prdts; i++)
    {
        volatile hba_prdt_entry_t* prdt = &slot.cmd_table->prdt_entry[i];

        size_t remaining = transfer_len - i * 0x3FFFFF;
        size_t transfer = remaining >= 0x3FFFFF ? 0x3FFFFF : remaining;

        uintptr_t pa = (uintptr_t)data + i * 0x3FFFFF;

        prdt->dbc = calc_bytecount(transfer);
        prdt->dba = pa & 0xFFFFFFFF;
        prdt->dbau = (pa >> 32) & 0xFFFFFFFF;

        prdt->i = 0;
    }

    wait_ready(dev);

    dev->port->ci |= (1 << slot.index);

    /* --- Real hw sits here forever --- */

    /* this has been suggested but doesnt work neither in real hw nor in vms
    while(!(dev->port->is & HBA_PxSI_DHRS))
        asm("pause");
    */

   // this only works on vms
    while(dev->port->ci & (1 << slot.index))
        asm("pause");

    free_command(dev, &slot);

    return true;
}