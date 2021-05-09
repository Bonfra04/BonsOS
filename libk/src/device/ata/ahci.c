#include <device/ata/ahci.h>
#include <panic.h>
#include <x86/ports.h>
#include <memory/page_frame_allocator.h>
#include <string.h>
#include <stdbool.h>
#include "ahci_types.h"

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

static void start_cmd(hba_port_t* port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);
 
    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
}

static int find_cmdslot(hba_port_t* port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++, slots >>= 1)
        if ((slots & 1) == 0)
            return i;
    return -1;
}

static bool ahci_identify(hba_device_t* hba_device)
{
    ahci_ident_t result;

    hba_device->port->is = -1; // Clear pending interrupt bits
    int slot = find_cmdslot(hba_device->port);

    if(slot == -1)
        return false;

    hba_cmd_header_t* cmd = &(hba_device->cmd_header[slot]);
    cmd->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd->w = 0;
    cmd->p = 1;
    cmd->c = 1;

    hba_cmd_table_t* cmdtable = (hba_cmd_table_t*)(((uint64_t)cmd->ctbau >> 32) | (uint64_t)cmd->ctba);
    memset(cmdtable, 0, sizeof(hba_cmd_table_t));

    cmdtable->prdt_entry[0].dba = (uint32_t)(uint64_t)&result;
    cmdtable->prdt_entry[0].dbau = ((uint64_t)&result >> 32);
    cmdtable->prdt_entry[0].dbc = 512;
    cmdtable->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)&(cmdtable->cfis);
    memset(cmdfis, 0, sizeof(fis_reg_h2d_t));
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->command = ATA_CMD_IDENTIFY;
    cmdfis->device = 0; // Master device
    cmdfis->c = 1;

    int spin = 0; // Spin lock timeout counter
    while ((hba_device->port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
        spin++;
    if (spin == 1000000)
        return false;

    hba_device->port->ci = 1 << slot;

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if (!(hba_device->port->ci & (1 << slot))) 
            break;
        if (hba_device->port->is & HBA_PxIS_TFES)   // Task file error
            return false;
    }

    // Check again
    if (hba_device->port->is & HBA_PxIS_TFES)
        return false;

    hba_device->lba_capacity = result.lba_capacity;
    return true;
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

    stop_cmd(device->port);

    device->port->clb = cmd_low;
    device->port->clbu = cmd_high;
    memset((void*)cmd_address, 0, 1024);

    device->port->fb = fis_low;
    device->port->fbu = fis_high;
    memset((void*)fis_address, 0, 256);

    device->cmd_header = (hba_cmd_header_t*)cmd_address;
    for (int i = 0; i < 32; i++)
    {
        device->cmd_header[i].prdtl = 1;

        uint64_t cmd_addr = i < 16 ? page1 : page2;
        cmd_addr += 256 * (i % 16);

        uint32_t cmd_addr_low = cmd_addr & 0xFFFFFFFFLL;
        uint32_t cmd_addr_high = (cmd_addr >> 32) & 0xFFFFFFFFLL;

        device->cmd_header[i].ctba = cmd_addr_low;
        device->cmd_header[i].ctbau = cmd_addr_high;
        memset((void*)cmd_addr, 0, 256);
    }

    start_cmd(device->port);

    ahci_identify(device);
}

static int check_type(hba_port_t* port)
{
    if ((port->ssts & 0x0F) != HBA_PORT_PRESENT)
        return AHCI_DEV_NULL;
    if (((port->ssts >> 8) & 0x0F) != HBA_PORT_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
    }

    return AHCI_DEV_NULL;
}

void ahci_init()
{
    registered_devices = 0;
}

size_t ahci_devices()
{
    return registered_devices;
}

size_t ahci_get_capacity(size_t device)
{
    if(device >= registered_devices)
        return -1;
    return devices[device].lba_capacity;
}

void ahci_register_pci_device(pci_device_t* device)
{
    uint32_t starting_device = registered_devices;

    hba_mem_t* hba_mem = (hba_mem_t*)(uint64_t)device->base5;

    for(int bit = 0; bit < 32; bit++)
        if(hba_mem->pi & (1 << bit)) // bit is set: device exists
        {
            hba_port_t* port = &(hba_mem->ports[bit]);
            
            switch (check_type(port))
            {
            case AHCI_DEV_SATA:
                {
                    if(registered_devices >= MAX_DEVICES)
                    kenrel_panic("Only %d AHCI devices are supported.", MAX_DEVICES);

                    hba_device_t hba_device;
                    hba_device.port = port;

                    devices[registered_devices] = hba_device;
                    configure_device(&(devices[registered_devices]));
                    registered_devices++;
                }
                break;
            }
        }

    if(starting_device != registered_devices)
    {
        uint64_t device_size = sizeof(hba_mem_t) + (registered_devices - starting_device) * sizeof(hba_port_t);
        pfa_deinit_region(device->base5, device_size);
    }
}

bool sata_read(size_t device, uint64_t lba, uint8_t count, void* address)
{
    if(device >= registered_devices)
        return false;

    hba_device_t* hba_device = &(devices[device]);

    hba_device->port->is = -1; // Clear pending interrupt bits
    int slot = find_cmdslot(hba_device->port);

    if(slot == -1)
        return false;

    hba_cmd_header_t* cmd = &(hba_device->cmd_header[slot]);
    cmd->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd->w = 0;

    hba_cmd_table_t* cmdtable = (hba_cmd_table_t*)(((uint64_t)cmd->ctbau >> 32) | (uint64_t)cmd->ctba);
    memset(cmdtable, 0, sizeof(hba_cmd_table_t) + (cmd->prdtl - 1) * sizeof(hba_prdt_entry_t));

    cmdtable->prdt_entry[0].dba = (uint32_t)(uint64_t)address;
    cmdtable->prdt_entry[0].dbau = ((uint64_t)address >> 32);
    cmdtable->prdt_entry[0].dbc = (count * 512) - 1;
    cmdtable->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)&(cmdtable->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    uint32_t lba_low = lba & 0xFFFFFFFFLL;
    uint32_t lba_high = (lba >> 32) & 0xFFFFFFFFLL;

    cmdfis->lba0 = (uint8_t)lba_low;
    cmdfis->lba1 = (uint8_t)(lba_low >> 8);
    cmdfis->lba2 = (uint8_t)(lba_low >> 16);
    cmdfis->device = 1 << 6;    // LBA mode
 
    cmdfis->lba3 = (uint8_t)(lba_low >> 24);
    cmdfis->lba4 = (uint8_t)lba_high;
    cmdfis->lba5 = (uint8_t)(lba_high >> 8);
 
    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    int spin = 0; // Spin lock timeout counter
    while ((hba_device->port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
        spin++;
    if (spin == 1000000)
        return false;

    hba_device->port->ci = 1 << slot;

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if (!(hba_device->port->ci & (1 << slot))) 
            break;
        if (hba_device->port->is & HBA_PxIS_TFES)   // Task file error
            return false;
    }

    // Check again
    if (hba_device->port->is & HBA_PxIS_TFES)
        return false;

    return true;
}

bool sata_write(size_t device, uint64_t lba, uint8_t count, void* address)
{
    if(device >= registered_devices)
        return false;

    hba_device_t* hba_device = &(devices[device]);

    hba_device->port->is = -1; // Clear pending interrupt bits
    int slot = find_cmdslot(hba_device->port);

    if(slot == -1)
        return false;

    hba_cmd_header_t* cmd = &(hba_device->cmd_header[slot]);
    cmd->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd->w = 1;

    hba_cmd_table_t* cmdtable = (hba_cmd_table_t*)(((uint64_t)cmd->ctbau >> 32) | (uint64_t)cmd->ctba);
    memset(cmdtable, 0, sizeof(hba_cmd_table_t) + (cmd->prdtl - 1) * sizeof(hba_prdt_entry_t));

    cmdtable->prdt_entry[0].dba = (uint32_t)(uint64_t)address;
    cmdtable->prdt_entry[0].dbau = ((uint64_t)address >> 32);
    cmdtable->prdt_entry[0].dbc = (count * 512) - 1;
    cmdtable->prdt_entry[0].i = 1;

    fis_reg_h2d_t* cmdfis = (fis_reg_h2d_t*)&(cmdtable->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;

    uint32_t lba_low = lba & 0xFFFFFFFFLL;
    uint32_t lba_high = (lba >> 32) & 0xFFFFFFFFLL;

    cmdfis->lba0 = (uint8_t)lba_low;
    cmdfis->lba1 = (uint8_t)(lba_low >> 8);
    cmdfis->lba2 = (uint8_t)(lba_low >> 16);
    cmdfis->device = 1 << 6;    // LBA mode
 
    cmdfis->lba3 = (uint8_t)(lba_low >> 24);
    cmdfis->lba4 = (uint8_t)lba_high;
    cmdfis->lba5 = (uint8_t)(lba_high >> 8);
 
    cmdfis->countl = count & 0xFF;
    cmdfis->counth = (count >> 8) & 0xFF;

    int spin = 0; // Spin lock timeout counter
    while ((hba_device->port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
        spin++;
    if (spin == 1000000)
        return false;

    hba_device->port->ci = 1 << slot;

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if (!(hba_device->port->ci & (1 << slot))) 
            break;
        if (hba_device->port->is & HBA_PxIS_TFES)   // Task file error
            return false;
    }

    // Check again
    if (hba_device->port->is & HBA_PxIS_TFES)
        return false;

    return true;
}
