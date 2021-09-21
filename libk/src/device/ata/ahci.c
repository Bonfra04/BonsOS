
#include "ahci_types.h"
#include <device/ata/ahci.h>
#include <memory/paging.h>
#include <memory/page_frame_allocator.h>
#include <panic.h>

static hba_device_t devices[MAX_DEVICES];
static size_t registered_devices;

static int check_type(volatile hba_port_t* port)
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

static uint8_t find_cmd_slot(hba_device_t* device)
{
    uint8_t cmd_slots = device->hba_mem->cap >> 8 & 0b11111;
    for(uint8_t i = 0; i < cmd_slots; i++)
        if(((device->port->sact | device->port->ci) & (1 << i)) == 0)
            return i;
    return -1;
}

static void send_cmd(hba_device_t* device, uint8_t cmd_slot)
{
    volatile hba_port_t* port = device->port;
    while((port->tfd & (0x80 | 0x8)));

    port->cmd &= ~HBA_PxCMD_ST;

    while(port->cmd & HBA_PxCMD_CR);

    port->cmd |= HBA_PxCMD_FRE | HBA_PxCMD_ST;
    port->ci = 1 << cmd_slot;

    while(port->ci & (1 << cmd_slot));

    port->cmd = port->cmd & ~HBA_PxCMD_ST;
    while(port->cmd & HBA_PxCMD_ST);
    port->cmd = port->cmd & ~HBA_PxCMD_FRE;
}

static inline void enable_ahci_mode(volatile hba_mem_t* hba_mem)
{
    hba_mem->ghc |= HBA_GHC_AE;
}

static void idle_ports(volatile hba_mem_t* hba_mem)
{
    for(uint8_t bit = 0; bit < 32; bit++)
        if(hba_mem->pi & (1 << bit)) // bit is set: device exists
        {
            volatile hba_port_t* port = &(hba_mem->ports[bit]);

            // make sure all ports are idle
            port->cmd &= ~HBA_PxCMD_ST;
            port->cmd &= ~HBA_PxCMD_CR;
            port->cmd &= ~HBA_PxCMD_FR;
            port->cmd &= ~HBA_PxCMD_FRE;
        }
}

static void standard_ahci_reset(volatile hba_mem_t* hba_mem)
{
    // standard AHCI reset
    hba_mem->ghc |= HBA_GHC_HR;
    while(hba_mem->ghc & HBA_GHC_HR)
        asm("pause");
}

static bool port_reset(volatile hba_port_t* port)
{
    uint64_t page0 = (uint64_t)pfa_alloc_page();
    memset((void*)page0, 0, pfa_page_size());

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

    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_SUD;

    // wait 1 ms
    pit_prepare_one_shot(100); 
    pit_wait_one_shot();

    uint64_t spin = 0;
    while((port->ssts & HBA_PxSSTS_DET) != 3 && spin < 1000000)
        spin++;

    if(spin >= 1000000)
        return false;

    port->serr = 0xFFFFFFFF;

    while(port->tfd & HBA_PxTFD_STS_DRQ && port->tfd & HBA_PxTFD_STS_BSY);

    port->is = 0;

    return true;
}

static bool ahci_identify(hba_device_t* hba_device)
{
    volatile ahci_ident_t result;

    uint8_t cmd_slot = find_cmd_slot(hba_device);
    if(cmd_slot == -1)
        return false;

    volatile hba_cmd_header_t* cmd_hdr = &(hba_device->cmd_header[cmd_slot]);
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_hdr->w = 0;
    cmd_hdr->prdtl = 1;

    volatile hba_cmd_table_t* cmd_table = (volatile hba_cmd_table_t*)(((uint64_t)cmd_hdr->ctbau >> 32) | (uint64_t)cmd_hdr->ctba);
    memset(cmd_table, 0, sizeof(hba_cmd_table_t));
    cmd_table->prdt_entry[0].dba = (uint32_t)((uint64_t)&result & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbau = (uint32_t)(((uint64_t)&result >> 32) & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbc = 511;
    cmd_table->prdt_entry[0].i = 1;

    volatile fis_reg_h2d_t* cmd = (volatile fis_reg_h2d_t*)(cmd_table->cfis);
    cmd->fis_type = FIS_TYPE_REG_H2D;
    cmd->command = ATA_CMD_IDENTIFY;
    cmd->device = 0;
    cmd->c = 1;
    cmd->pmport = 0;

    send_cmd(hba_device, cmd_slot);

     hba_device->lba_capacity = result.lba_capacity;

    for(int i = 0; i < 40/2; i++)
        if(((uint16_t*)result.model)[i])
        {
            char c1 = ((uint16_t*)result.model)[i];
            char c2 = ((uint16_t*)result.model)[i] >> 8;
            printf("%c%c", c2, c1);
        }
    printf("\n");

    return true;
}

static bool configure_device(hba_device_t* device)
{
    uint64_t page1 = (uint64_t)pfa_alloc_page();
    uint64_t page2 = (uint64_t)pfa_alloc_page();
    memset((void*)page1, 0, pfa_page_size());
    memset((void*)page2, 0, pfa_page_size());

    uint64_t cmd_address = (uint64_t)device->port->clb | ((uint64_t)device->port->clbu >> 32ull);
    device->cmd_header = (hba_cmd_header_t*)cmd_address;

    for (int i = 0; i < 32; i++)
    {
        device->cmd_header[i].prdtl = 1;

        uint64_t cmd_addr = i < 16 ? page1 : page2;
        cmd_addr += 256 * (i % 16);

        uint32_t cmd_addr_low = cmd_addr & 0xFFFFFFFFull;
        uint32_t cmd_addr_high = (cmd_addr >> 32) & 0xFFFFFFFFull;

        device->cmd_header[i].ctba = cmd_addr_low;
        device->cmd_header[i].ctbau = cmd_addr_high;
    }

    return ahci_identify(device);
}

void ahci_init()
{
    registered_devices = 0;
}

void ahci_register_pci_device(const pci_device_t* device)
{
    uint32_t starting_device = registered_devices;

    volatile hba_mem_t* hba_mem = (volatile hba_mem_t*)(uint64_t)device->base5;
    extern paging_data_t kernel_paging;
    paging_map(kernel_paging, hba_mem, hba_mem, sizeof(hba_mem_t), PAGE_PRIVILEGE_KERNEL);

    enable_ahci_mode(hba_mem);

    idle_ports(hba_mem);

    standard_ahci_reset(hba_mem);

    enable_ahci_mode(hba_mem);

    for(int bit = 0; bit < 32; bit++)
        if(hba_mem->pi & (1 << bit)) // bit is set: device exists
        {
            volatile hba_port_t* port = &(hba_mem->ports[bit]);
            bool device_connted = port_reset(port);

            if(!device_connted)
                continue;

            switch (check_type(port))
            {
            case AHCI_DEV_SATA:
                {
                    if(registered_devices >= MAX_DEVICES)
                        kenrel_panic("Only %d AHCI devices are supported.", MAX_DEVICES);

                    hba_device_t hba_device;
                    hba_device.port = port;
                    hba_device.hba_mem = hba_mem;

                    bool configured = configure_device(&hba_device);
                    if(configured)
                    {
                        devices[registered_devices] = hba_device;
                        registered_devices++;
                    }
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

inline size_t ahci_devices()
{
    return registered_devices;
}

inline size_t ahci_get_capacity(size_t device)
{
    return device >= registered_devices ? -1 : devices[device].lba_capacity;
}

static bool ahci_rw(size_t device, uint64_t lba, size_t count, void* address, bool write)
{
    if(device >= registered_devices)
        return false;
    hba_device_t* hba_device = &(devices[device]);

    uint8_t cmd_slot = find_cmd_slot(hba_device);
    if(cmd_slot == -1)
        return;

    volatile hba_cmd_header_t* cmd_hdr = &(hba_device->cmd_header[cmd_slot]);
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_hdr->w = 0;
    cmd_hdr->prdtl = 1;

    volatile hba_cmd_table_t* cmd_table = (volatile hba_cmd_table_t*)(((uint64_t)cmd_hdr->ctbau >> 32) | (uint64_t)cmd_hdr->ctba);
    memset(cmd_table, 0, sizeof(hba_cmd_table_t));
    cmd_table->prdt_entry[0].dba = (uint32_t)((uint64_t)address & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbau = (uint32_t)(((uint64_t)address >> 32) & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbc = (uint64_t)count * 512 - 1;
    cmd_table->prdt_entry[0].i = 1;

    volatile fis_reg_h2d_t* cmd = (volatile fis_reg_h2d_t*)(cmd_table->cfis);
    cmd->fis_type = FIS_TYPE_REG_H2D;
    cmd->c = 1;
    cmd->device = 1 << 6;
    cmd->command = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;

    uint32_t lba_low = lba & 0xFFFFFFFFLL;
    uint32_t lba_high = (lba >> 32) & 0xFFFFFFFFLL;

    cmd->lba0 = lba_low & 0xFF;
    cmd->lba1 = lba_low >> 8 & 0xFF;
    cmd->lba2 = lba_low >> 16 & 0xFF;
    cmd->lba3 = lba_low >> 24 & 0xFF;
    cmd->lba4 = lba_high & 0xFF;
    cmd->lba5 = lba_high >> 8 & 0xFF;
 
    cmd->countl = count & 0xFF;
    cmd->counth = count >> 8 & 0xFF;

    send_cmd(hba_device, cmd_slot);

    return true;
}

bool ahci_read(size_t device, uint64_t lba, uint8_t count, void* address)
{
    return ahci_rw(device, lba, count, address, false);
}

bool ahci_write(size_t device, uint64_t lba, uint8_t count, void* address)
{
    return ahci_rw(device, lba, count, address, true);
}