#include <device/ata/sata.h>
#include <panic.h>
#include <x86/ports.h>

#define MAX_DEVICES 4

#define ATA_DATA 0x0
#define ATA_ERROR 0x1
#define ATA_FEATURES 0x1
#define ATA_SECT_CNT 0x2
#define ATA_SECT_NUM 0x3
#define ATA_LBA_LOW 0x03
#define ATA_CYL_LOW 0x04
#define ATA_LBA_MID 0x4
#define ATA_CYL_HIGH 0x5
#define ATA_LBA_HIGH 0x5
#define ATA_DRV_HEAD 0x6
#define ATA_STATUS 0x7
#define ATA_COMMAND 0x7

#define ATA_ALT_STAT 0x2
#define ATA_DEV_CNTR 0x2
#define ATA_DEV_ADDR 0x3

typedef struct
{
    uint32_t clb;       // 0x00, command list base address, 1K-byte aligned
    uint32_t clbu;      // 0x04, command list base address upper 32 bits
    uint32_t fb;        // 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;       // 0x0C, FIS base address upper 32 bits
    uint32_t is;        // 0x10, interrupt status
    uint32_t ie;        // 0x14, interrupt enable
    uint32_t cmd;       // 0x18, command and status
    uint32_t rsv0;      // 0x1C, Reserved
    uint32_t tfd;       // 0x20, task file data
    uint32_t sig;       // 0x24, signature
    uint32_t ssts;      // 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;      // 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;      // 0x30, SATA error (SCR1:SError)
    uint32_t sact;      // 0x34, SATA active (SCR3:SActive)
    uint32_t ci;        // 0x38, command issue
    uint32_t sntf;      // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;       // 0x40, FIS-based switch control
    uint32_t rsv1[11];  // 0x44 ~ 0x6F, Reserved
    uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific
} hba_port_t __attribute__ ((packed));

typedef struct
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
 
    uint8_t rsv[0x74];
    uint8_t vendor[0x60];
 
    hba_port_t ports[];    // 1 ~ 32
} hba_mem_t __attribute__ ((packed));

typedef struct
{
    uint32_t abar;
    hba_mem_t* hba_mem;
} ahci_device_t;

static ahci_device_t devices[MAX_DEVICES];
static size_t registered_devices;

void sata_init()
{
    registered_devices = 0;
}

size_t sata_devices()
{
    return registered_devices;
}

void sata_register_device(pci_device_t* device)
{
    if(registered_devices >= MAX_DEVICES)
        kenrel_panic("Only %d SATA are supported.", MAX_DEVICES);

    /* init */
    ahci_device_t ahci_device;
    ahci_device.abar = device->base5;
    ahci_device.hba_mem = (void*)device->base5;

    devices[registered_devices++] = ahci_device;
}

void sata_read(size_t device, uint64_t lba, uint8_t count, void* address)
{

}

void sata_write(size_t device, uint64_t lba, uint8_t count, void* address)
{

}
