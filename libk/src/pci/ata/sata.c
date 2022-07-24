#include <pci/ata/sata.h>
#include <memory/paging.h>
#include <memory/pfa.h>
#include <pit.h>
#include <log.h>

#include <linker.h>
#include <containers/darray.h>

#include <string.h>

#include "sata_types.h"

static sata_device_t* sata_devices;

void sata_init()
{
    sata_devices = darray(sata_device_t, 0);
}

static void enable_ahci_mode(volatile hba_mem_t* hba)
{
    hba->ghc |= HBA_GHC_AE;
}

static void bios_handoff(volatile hba_mem_t* hba)
{
    if(!(hba->cap2 & HBA_CAP2_BOH))
        return;

    hba->bohc |= HBA_BOHC_OOS;
    while(!(hba->bohc & HBA_BOHC_OOS) || (hba->bohc & HBA_BOHC_BOS))
        asm("pause");
}

static void idle_ports(volatile hba_mem_t* hba)
{
    for(uint8_t bit = 0; bit < 32; bit++)
        if(hba->pi & (1 << bit)) // bit is set device exists
        {
            volatile hba_port_t* port = &hba->ports[bit];
            
            port->cmd &= ~HBA_PxCMD_ST;
            port->cmd &= ~HBA_PxCMD_CR;
            port->cmd &= ~HBA_PxCMD_FR;
            port->cmd &= ~HBA_PxCMD_FRE;
        }
}

static void ahci_reset(volatile hba_mem_t* hba)
{
    hba->ghc |= HBA_GHC_HR;
    while(hba->ghc & HBA_GHC_HR)
        asm("pause");
}

static bool init_port(volatile hba_port_t* port)
{
    uint64_t page0 = (uint64_t)pfa_alloc(1);
    memset((void*)page0, 0, PFA_PAGE_SIZE);

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

    pit_prepare_one_shot(1); 
    pit_perform_one_shot();

    uint64_t spin = 0;
    while((port->ssts & HBA_PxSSTS_DET) != 3 && spin < 1000000)
        spin++;

    if(spin >= 1000000)
    {
        // kernel_log("HERE 1\n");
        return false;
    }

    port->serr = 0xFFFFFFFF;

    spin = 0;
    while(port->tfd & HBA_PxTFD_STS_DRQ && port->tfd & HBA_PxTFD_STS_BSY && spin < 1000000)
        spin++;

    if(spin >= 1000000)
    {
        kernel_log("HERE 2\n");
        return false;
    }

    port->is = 0;

    volatile hba_cmd_header_t* cmd_header = ptr(cmd_address);
    uint64_t page1 = (uint64_t)pfa_alloc(1);
    uint64_t page2 = (uint64_t)pfa_alloc(1);
    memset((void*)page1, 0, PFA_PAGE_SIZE);
    memset((void*)page2, 0, PFA_PAGE_SIZE);

    for (uint8_t i = 0; i < 32; i++)
    {
        cmd_header[i].prdtl = 1;
        cmd_header[i].pmp = 0;
        cmd_header[i].c = true;
        cmd_header[i].b = false;
        cmd_header[i].r = false;
        cmd_header[i].p = false;
        cmd_header[i].a = false;

        uint64_t cmd_addr = i < 16 ? page1 : page2;
        cmd_addr += 256 * (i % 16);

        uint32_t cmd_addr_low = cmd_addr & 0xFFFFFFFFull;
        uint32_t cmd_addr_high = (cmd_addr >> 32) & 0xFFFFFFFFull;

        cmd_header[i].ctba = cmd_addr_low;
        cmd_header[i].ctbau = cmd_addr_high;
    }

    return true;
}

static bool identify(sata_device_t* device);

static void init_device(volatile hba_mem_t* hba)
{
    enable_ahci_mode(hba);
    bios_handoff(hba);
    idle_ports(hba);
    ahci_reset(hba);
    enable_ahci_mode(hba);
    
    for(uint8_t bit = 0; bit < 32; bit++)
        if(hba->pi & (1 << bit)) // bit is set device exists
        {
            volatile hba_port_t* port = &hba->ports[bit];
            paging_map(NULL, (void*)port, (void*)port, sizeof(hba_port_t), PAGE_PRIVILEGE_KERNEL);
            pfa_deinit_region((void*)port, sizeof(hba_port_t));

            if(!init_port(port))
                continue;

            sata_device_t device;
            device.mem = hba;
            device.port = port;
            device.cmd_header = ptr((uint64_t)port->clb | ((uint64_t)port->clbu << 32ull));

            if(!identify(&device))
                continue;

            darray_append(sata_devices, device);
        }
}

static uint8_t find_cmd_slot(sata_device_t* device)
{
    uint8_t cmd_slots = device->mem->cap >> 8 & 0b11111;
    for(uint8_t i = 0; i < cmd_slots; i++)
        if(((device->port->sact | device->port->ci) & (1 << i)) == 0)
            return i;
    return UINT8_MAX;
}

static void send_cmd(sata_device_t* device, uint8_t cmd_slot)
{
    volatile hba_port_t* port = device->port;
    while((port->tfd & (0x80 | 0x8)))
        asm("pause");

    port->cmd &= ~HBA_PxCMD_ST;

    while(port->cmd & HBA_PxCMD_CR)
        asm("pause");

    port->cmd |= HBA_PxCMD_FRE | HBA_PxCMD_ST;
    port->ci = 1 << cmd_slot;

    while(port->ci & (1 << cmd_slot))
        asm("pause");

    port->cmd = port->cmd & ~HBA_PxCMD_ST;
    while(port->cmd & HBA_PxCMD_ST)
        asm("pause");

    port->cmd = port->cmd & ~HBA_PxCMD_FRE;
}

static bool identify(sata_device_t* device)
{   
    volatile ahci_ident_t* result = &device->ident;

    uint8_t cmd_slot = find_cmd_slot(device);
    if(cmd_slot == UINT8_MAX)
        return false;

    volatile hba_cmd_header_t* cmd_hdr = &(device->cmd_header[cmd_slot]);
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_hdr->w = 0;

    volatile hba_cmd_table_t* cmd_table = (volatile hba_cmd_table_t*)(((uint64_t)cmd_hdr->ctbau << 32) | (uint64_t)cmd_hdr->ctba);
    memset((void*)cmd_table, 0, sizeof(hba_cmd_table_t));
    cmd_table->prdt_entry[0].dba = (uint32_t)((uint64_t)result & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbau = (uint32_t)(((uint64_t)result >> 32) & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbc = sizeof(ahci_ident_t) - 1;
    cmd_table->prdt_entry[0].i = false;

    volatile fis_reg_h2d_t* cmd = (volatile fis_reg_h2d_t*)(cmd_table->cfis);
    memset((void*)cmd, 0, sizeof(fis_reg_h2d_t));
    cmd->fis_type = FIS_TYPE_REG_H2D;
    cmd->c = 1;
    cmd->command = ATA_CMD_IDENTIFY;

    send_cmd(device, cmd_slot);

    return true;
}

static bool ahci_rw(sata_device_t* device, uint64_t lba, void* address, size_t sectors, bool write)
{
    uint8_t cmd_slot = find_cmd_slot(device);
    if(cmd_slot == UINT8_MAX)
        return false;

    volatile hba_cmd_header_t* cmd_hdr = &(device->cmd_header[cmd_slot]);
    cmd_hdr->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmd_hdr->w = 0;

    volatile hba_cmd_table_t* cmd_table = (volatile hba_cmd_table_t*)(((uint64_t)cmd_hdr->ctbau << 32) | (uint64_t)cmd_hdr->ctba);
    memset((void*)cmd_table, 0, sizeof(hba_cmd_table_t));
    cmd_table->prdt_entry[0].dba = (uint32_t)((uint64_t)address & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbau = (uint32_t)(((uint64_t)address >> 32) & 0xFFFFFFFF);
    cmd_table->prdt_entry[0].dbc = sectors * 512 - 1;
    cmd_table->prdt_entry[0].i = false;

    volatile fis_reg_h2d_t* cmd = (volatile fis_reg_h2d_t*)(cmd_table->cfis);
    memset((void*)cmd, 0, sizeof(fis_reg_h2d_t));
    cmd->fis_type = FIS_TYPE_REG_H2D;
    cmd->c = 1;
    cmd->command = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;
    cmd->device = 1 << 6; // TODO: why?

    uint32_t lba_low = lba & 0xFFFFFFFFLL;
    uint32_t lba_high = (lba >> 32) & 0xFFFFFFFFLL;

    cmd->lba0 = lba_low & 0xFF;
    cmd->lba1 = (lba_low >> 8) & 0xFF;
    cmd->lba2 = (lba_low >> 16) & 0xFF;
    cmd->lba3 = (lba_low >> 24) & 0xFF;
    cmd->lba4 = lba_high & 0xFF;
    cmd->lba5 = (lba_high >> 8) & 0xFF;
 
    cmd->countl = sectors & 0xFF;
    cmd->counth = (sectors >> 8) & 0xFF;

    send_cmd(device, cmd_slot);

    return true;
}

void sata_register_device(const pci_device_t* pci_device)
{
    paging_map(NULL, ptr(pci_device->base5), ptr(pci_device->base5), sizeof(hba_mem_t), PAGE_PRIVILEGE_KERNEL);
    pfa_deinit_region(ptr(pci_device->base5), sizeof(hba_mem_t));

    init_device((volatile void*)ptr(pci_device->base5));
}

size_t sata_ndisks()
{
    return darray_length(sata_devices);
}

bool sata_read(size_t disk, uint64_t lba, size_t sectors, void* buffer)
{
    return ahci_rw(&sata_devices[disk], lba, buffer, sectors, false);
}

bool sata_write(size_t disk, uint64_t lba, size_t sectors, void* buffer)
{
    return ahci_rw(&sata_devices[disk], lba, buffer, sectors, true);
}

size_t sata_get_capacity(size_t disk)
{
    return sata_devices[disk].ident.lba_capacity * sata_devices[disk].ident.sector_bytes;
}

size_t sata_get_sector_size(size_t disk)
{
    return sata_devices[disk].ident.sector_bytes;
}

void sata_get_model(size_t disk, char* model)
{
    uint16_t* m = (uint16_t*)sata_devices[disk].ident.model;

    memset(model, '\'', 40);
    for(uint8_t i = 0; i < 20; i++)
        if(m[i])
        {
            char c1 = m[i];
            char c2 = m[i] >> 8;
            model[i * 2] = c2;
            model[i * 2 + 1] = c1;
        }
}
