#include <drivers/storage/ahci.h>
#include <drivers/storage/ata.h>
#include <memory/paging.h>
#include <memory/pfa.h>
#include <timers/pit.h>
#include <log.h>
#include <panic.h>
#include <alignment.h>

#include <linker.h>

#include <string.h>
#include <stdlib.h>

#include "ahci_types.h"

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

    return UINT8_MAX;
}

static ahci_command_t allocate_command(ahci_device_t* dev)
{
    uint8_t i = get_free_cmd_slot(dev);
    if(i == UINT8_MAX)
        return (ahci_command_t){ .cmd_table = NULL, .index = UINT8_MAX };

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

static void wait_busy(volatile hba_port_t* port)
{
    while(port->tfd & (HBA_PxTFD_STS_BSY | HBA_PxTFD_STS_DRQ))
        asm("pause");
}

static void idle_port(volatile hba_port_t* port)
{
    port->cmd &= ~HBA_PxCMD_ST;
    while(port->cmd & HBA_PxCMD_CR)
        asm("pause");

    if(port->cmd & HBA_PxCMD_FRE)
    {
        port->cmd &= ~HBA_PxCMD_FRE;
        while(port->cmd &HBA_PxCMD_FR)
            asm("pause");
    }
}

static bool reset_port(volatile hba_port_t* port)
{
    port->cmd |= HBA_PxCMD_FRE;

    port->sctl |= HBA_PxSCTL_DET_RESET;
    pit_prepare_one_shot(1);
    pit_perform_one_shot();
    port->sctl &= ~HBA_PxSCTL_DET_RESET;

    uint64_t spin = 0;
    while((port->ssts & HBA_PxSSTS_DET) != HBA_PxSSTS_DET_DEV_PHY && spin++ < 1000000)
        asm("pause");
    if(spin > 1000000)
        return false;

    port->serr = UINT32_MAX;

    while(port->tfd & HBA_PxTFD_STS_BSY)
        asm("pause");

    port->ie = 0;
    port->is = UINT32_MAX;

    return true;
}

static bool init_port(volatile hba_port_t* port)
{
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

    if(!reset_port(port))
        return false;

    if((port->ssts & HBA_PxSSTS_DET) != 3)
        return false;

    port->serr &= ~HBA_PxSERR_DIAG_X;
    wait_busy(port);

    while(port->cmd & HBA_PxCMD_CR)
        asm("pause");
        
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;

    return true;
}

static void bios_handoff(volatile hba_mem_t* hba)
{
    if(!(hba->cap2 & HBA_CAP2_BOH))
        return;

    hba->bohc |= HBA_BOHC_OOS;
    while(!(hba->bohc & HBA_BOHC_OOS) || (hba->bohc & HBA_BOHC_BOS))
        asm("pause");
}

static void hba_reset(volatile hba_mem_t* hba)
{
    hba->ghc = HBA_GHC_AE;
    hba->ghc |= HBA_GHC_HR;
    while(hba->ghc & HBA_GHC_HR)
        asm("pause");
    hba->ghc = HBA_GHC_AE;
}

static bool send_ata_cmd(void* device, ata_command_t* command, uint8_t* data, size_t transfer_len)
{
    ahci_device_t* dev = (ahci_device_t*)device;

    dev->port->is = UINT32_MAX;
 
    size_t n_prdts = (transfer_len + 0x3FFFFF - 1) / 0x3FFFFF;
    ahci_command_t slot = allocate_command(dev);

    if(slot.index == UINT8_MAX)
        return false;

    dev->cmd_header[slot.index].prdtl = n_prdts;
    dev->cmd_header[slot.index].write = command->write;
    dev->cmd_header[slot.index].cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    dev->cmd_header[slot.index].a = 0;

    volatile fis_reg_h2d_t* fis = (void*)slot.cmd_table->cfis;
    memset((void*)fis, 0, sizeof(fis_reg_h2d_t));
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

    wait_busy(dev->port);

    dev->port->ci = 1 << slot.index;
    
    bool success = true;
    while(dev->port->ci & (1 << slot.index))
        if((dev->port->is & HBA_PxSI_TFES))
        {
            success = false;
            break;
        }
    if(success == true)
        success = !(dev->port->is & HBA_PxSI_TFES);
    free_command(dev, &slot);

    return success;
}

static bool send_atapi_cmd(void* device, atapi_command_t* command, uint8_t* data, size_t transfer_len)
{
    ahci_device_t* dev = (ahci_device_t*)device;

    dev->port->is = UINT32_MAX;
 
    size_t n_prdts = (transfer_len + 0x3FFFFF - 1) / 0x3FFFFF;
    ahci_command_t slot = allocate_command(dev);

    if(slot.index == UINT8_MAX)
        return false;

    dev->cmd_header[slot.index].prdtl = n_prdts;
    dev->cmd_header[slot.index].write = command->write;
    dev->cmd_header[slot.index].cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    dev->cmd_header[slot.index].a = 1;

    volatile fis_reg_h2d_t* fis = (void*)slot.cmd_table->cfis;
    memset((void*)fis, 0, sizeof(fis_reg_h2d_t));
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;

    fis->command = ATA_CMD_SEND_PACKET;
    fis->control = 0x8;
    fis->device = 0xA0;

    fis->lba0 = transfer_len & 0xFF;
    fis->lba1 = (transfer_len >> 8) & 0xFF;

    fis->featurel = 1;

    memcpy((void*)slot.cmd_table->acmd, command->packet, 16);

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

    wait_busy(dev->port);

    dev->port->ci = 1 << slot.index;
    
    bool success = true;
    while(dev->port->ci & (1 << slot.index))
        if((dev->port->is & HBA_PxSI_TFES))
        {
            success = false;
            break;
        }
    if(success == true)
        success = !(dev->port->is & HBA_PxSI_TFES);
    free_command(dev, &slot);

    return success;
}

static void init_device(volatile hba_mem_t* hba)
{
    hba_reset(hba);
    hba->is = UINT32_MAX; // clear interrupt status
    bios_handoff(hba);

    for(uint8_t bit = 0; bit < 32; bit++)
        if(hba->pi & (1 << bit)) // bit is set device exists
        {
            volatile hba_port_t* port = &hba->ports[bit];
            paging_map(NULL, (void*)port, (void*)port, sizeof(hba_port_t), PAGE_PRIVILEGE_KERNEL);
            pfa_deinit_region((void*)port, sizeof(hba_port_t));

            if(!init_port(port))
                continue;

            ahci_device_t* ahci_device = malloc(sizeof(ahci_device_t));
            ahci_device->port = port;
            ahci_device->allocated_cmd = 0;
            ahci_device->ncmd = ((hba->cap >> 8) & HBA_CAP_NCS) + 1;
            ahci_device->cmd_header = (void*)(port->clb | ((uint64_t)port->clbu << 32));

            ata_driver_t ata_driver;
            ata_driver.data = ahci_device;
            ata_driver.send_ata_cmd = send_ata_cmd;
            ata_driver.send_atapi_cmd = send_atapi_cmd;
            ata_driver.atapi = port->sig == AHCI_SIG_ATAPI;

            ata_register_device(ata_driver);
        }
}

static void register_controller(const pci_dev_info_t* pci_device)
{
    pci_set_privileges(pci_device, PCI_PRIV_DMA | PCI_PRIV_MMIO);

    paging_map(NULL, ptr(pci_device->dev.base5), ptr(pci_device->dev.base5), sizeof(hba_mem_t), PAGE_PRIVILEGE_KERNEL);
    pfa_deinit_region(ptr(pci_device->dev.base5), sizeof(hba_mem_t));

    init_device((volatile void*)ptr(pci_device->dev.base5));
}

#define PCI_CLASS_ATA 0x01
#define PCI_ATA_SUB_SATA 0x06

static pci_driver_t driver = {
    .match = PCI_DRIVER_MATCH_CLASS | PCI_DRIVER_MATCH_SUBCLASS,
    .class = PCI_CLASS_ATA,
    .subclass = PCI_ATA_SUB_SATA,
    .register_device = register_controller
};

void ahci_init()
{
    pci_register_driver(&driver);
}
