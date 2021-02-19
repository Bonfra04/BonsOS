#include <device/ata/sata.h>
#include <panic.h>
#include <x86/ports.h>
#include <memory/page_frame_allocator.h>
#include <string.h>
#include <stdbool.h>
#include "sata_types.h"

#define MAX_DEVICES 32

#define HBA_PI_ACTIVE (1 << 0)
#define HBA_PI_PRESENT (3 << 8)
 
#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

#define ATA_DEV_BUSY    (1 << 7)
#define ATA_DEV_DRQ     (1 << 3)

#define HBA_PxIS_TFES   (1 << 30)

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

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
        cmdheader[i].prdtl = 1;

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
            configure_device(&(devices[i]));
    }
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

    hba_cmd_table_t* cmdtable = (hba_cmd_table_t*)cmd->ctba;
    memset(cmdtable, 0, sizeof(hba_cmd_table_t) + (cmd->prdtl - 1) * sizeof(hba_prdt_entry_t));

    cmdtable->prdt_entry[0].dba = (uint32_t)address;
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

}
