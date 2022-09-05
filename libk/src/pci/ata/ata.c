#include <pci/ata/ata.h>
#include <pci/pci.h>
#include <log.h>
#include <pci/ata/ahci.h>

#include <containers/darray.h>

#include <string.h>

#define PCI_ATA_SUB_SCSI 0x00
#define PCI_ATA_SUB_IDE 0x01
#define PCI_ATA_SUB_FDC 0x02
#define PCI_ATA_SUB_IPI 0x03
#define PCI_ATA_SUB_RAID 0x04
#define PCI_ATA_SUB_ATA 0x05
#define PCI_ATA_SUB_SATA 0x06
#define PCI_ATA_SUB_SAS 0x07
#define PCI_ATA_SUB_SSS 0x08
#define PCI_ATA_SUB_OTHER 0x80

typedef struct ata_ident
{
    uint16_t config;                /* lots of obsolete bit flags */
    uint16_t cyls;                  /* obsolete */
    uint16_t reserved2;             /* special config */
    uint16_t heads;                 /* "physical" heads */
    uint16_t track_bytes;           /* unformatted bytes per track */
    uint16_t sector_bytes;          /* unformatted bytes per sector */
    uint16_t sectors;               /* "physical" sectors per track */
    uint16_t vendor0;               /* vendor unique */
    uint16_t vendor1;               /* vendor unique */
    uint16_t vendor2;               /* vendor unique */
    uint8_t  serial_no[20];         /* 0 = not_specified */
    uint16_t buf_type;              /*  */
    uint16_t buf_size;              /* 512 byte increments; 0 = not_specified */
    uint16_t ecc_bytes;             /* for r/w long cmds; 0 = not_specified */
    uint8_t  fw_rev[8];             /* 0 = not_specified */
    uint8_t  model[40];             /* 0 = not_specified */
    uint16_t multi_count;           /* Multiple Count */
    uint16_t dword_io;              /* 0=not_implemented; 1=implemented */
    uint16_t capability1;           /* vendor unique */
    uint16_t capability2;           /* bits 0:DMA 1:LBA 2:IORDYsw 3:IORDYsup word: 50 */
    uint8_t  vendor5;               /* vendor unique */
    uint8_t  tPIO;                  /* 0=slow, 1=medium, 2=fast */
    uint8_t  vendor6;               /* vendor unique */
    uint8_t  tDMA;                  /* 0=slow, 1=medium, 2=fast */
    uint16_t field_valid;           /* bits 0:cur_ok 1:eide_ok */
    uint16_t cur_cyls;              /* logical cylinders */
    uint16_t cur_heads;             /* logical heads word 55*/
    uint16_t cur_sectors;           /* logical sectors per track */
    uint16_t cur_capacity0;         /* logical total sectors on drive */
    uint16_t cur_capacity1;         /*  (2 words, misaligned int)     */
    uint8_t  multsect;              /* current multiple sector count */
    uint8_t  multsect_valid;        /* when (bit0==1) multsect is ok */
    uint32_t lba_capacity;          /* total number of sectors */
    uint16_t dma_1word;             /* single-word dma info */
    uint16_t dma_mword;             /* multiple-word dma info */
    uint16_t eide_pio_modes;        /* bits 0:mode3 1:mode4 */
    uint16_t eide_dma_min;          /* min mword dma cycle time (ns) */
    uint16_t eide_dma_time;         /* recommended mword dma cycle time (ns) */
    uint16_t eide_pio;              /* min cycle time (ns), no IORDY  */
    uint16_t eide_pio_iordy;        /* min cycle time (ns), with IORDY */
    uint16_t words69_70[2];         /* reserved words 69-70 */
    uint16_t words71_74[4];         /* reserved words 71-74 */
    uint16_t queue_depth;           /*  */
    uint16_t sata_capability;       /*  SATA Capabilities word 76*/
    uint16_t sata_additional;       /*  Additional Capabilities */
    uint16_t sata_supported;        /* SATA Features supported  */
    uint16_t features_enabled;      /* SATA features enabled */
    uint16_t major_rev_num;         /*  Major rev number word 80 */
    uint16_t minor_rev_num;         /*  */
    uint16_t command_set_1;         /* bits 0:Smart 1:Security 2:Removable 3:PM */
    uint16_t command_set_2;   /*83*//* bits 14:Smart Enabled 13:0 zero */
    uint16_t cfsse;                 /* command set-feature supported extensions */
    uint16_t cfs_enable_1;          /* command set-feature enabled */
    uint16_t cfs_enable_2;          /* command set-feature enabled */
    uint16_t csf_default;           /* command set-feature default */
    uint16_t dma_ultra;             /*  */
    uint16_t word89;                /* reserved (word 89) */
    uint16_t word90;                /* reserved (word 90) */
    uint16_t CurAPMvalues;          /* current APM values */
    uint16_t word92;                /* reserved (word 92) */
    uint16_t comreset;              /* should be cleared to 0 */
    uint16_t accoustic;             /* accoustic management */
    uint16_t min_req_sz;            /* Stream minimum required size */
    uint16_t transfer_time_dma;     /* Streaming Transfer Time-DMA */
    uint16_t access_latency;        /* Streaming access latency-DMA & PIO WORD 97*/
    uint32_t perf_granularity;      /* Streaming performance granularity */
    uint32_t total_usr_sectors[2];  /* Total number of user addressable sectors */
    uint16_t transfer_time_pio;     /* Streaming Transfer time PIO */
    uint16_t reserved105;           /* Word 105 */
    uint16_t sector_sz;             /* Physical Sector size / Logical sector size */
    uint16_t inter_seek_delay;      /* In microseconds */
    uint16_t words108_116[9];       /*  */
    uint32_t words_per_sector;      /* words per logical sectors */
    uint16_t supported_settings;    /* continued from words 82-84 */
    uint16_t command_set_3;         /* continued from words 85-87 */
    uint16_t words121_126[6];       /* reserved words 121-126 */
    uint16_t word127;               /* reserved (word 127) */
    uint16_t security_status;       /* device lock function */
    uint16_t csfo;                  /* current set features options */
    uint16_t words130_155[26];      /* reserved vendor words 130-155 */
    uint16_t word156;
    uint16_t words157_159[3];       /* reserved vendor words 157-159 */
    uint16_t cfa;                   /* CFA Power mode 1 */
    uint16_t words161_175[15];      /* Reserved */
    uint8_t  media_serial[60];      /* words 176-205 Current Media serial number */
    uint16_t sct_cmd_transport;     /* SCT Command Transport */
    uint16_t words207_208[2];       /* reserved */
    uint16_t block_align;           /* Alignment of logical blocks in larger physical blocks */
    uint32_t WRV_sec_count;         /* Write-Read-Verify sector count mode 3 only */
    uint32_t verf_sec_count;        /* Verify Sector count mode 2 only */
    uint16_t nv_cache_capability;   /* NV Cache capabilities */
    uint16_t nv_cache_sz;           /* NV Cache size in logical blocks */
    uint16_t nv_cache_sz2;          /* NV Cache size in logical blocks */
    uint16_t rotation_rate;         /* Nominal media rotation rate */
    uint16_t reserved218;           /*  */
    uint16_t nv_cache_options;      /* NV Cache options */
    uint16_t words220_221[2];       /* reserved */
    uint16_t transport_major_rev;   /*  */
    uint16_t transport_minor_rev;   /*  */
    uint16_t words224_233[10];      /* Reserved */
    uint16_t min_dwnload_blocks;    /* Minimum number of 512byte units per DOWNLOAD MICROCODE command for mode 03h */
    uint16_t max_dwnload_blocks;    /* Maximum number of 512byte units per DOWNLOAD MICROCODE command for mode 03h */
    uint16_t words236_254[19];      /* Reserved */
    uint16_t integrity;             /* Checksum, Signature */
} __attribute__ ((packed)) ata_ident_t;

#define ATA_IDENT_CMD2_48BIT (1 << 10)
#define ATA_IDENT_CMD3_48BIT (1 << 10)

static ata_device_t* ata_devices;

static bool identify(ata_device_t* dev)
{
    ata_ident_t ident;

    ata_command_t cmd;
    memset(&cmd, 0, sizeof(ata_command_t));
    cmd.command = ATA_CMD_IDENTIFY;

    if(!dev->send_ata_cmd(dev->internal_id, &cmd, (void*)&ident, sizeof(ata_ident_t)))
        return false;

    dev->lba48 = (ident.command_set_2 & ATA_IDENT_CMD2_48BIT) && (ident.command_set_3 & ATA_IDENT_CMD3_48BIT);

    uint64_t sector_size = ident.sector_sz;
    if(sector_size & (1 << 14) && !(sector_size & (1 << 15)) // field contains valid info
    && sector_size & (1 << 12)) // logical sectors are larger than 512
        dev->sector_size = ident.words_per_sector;
    else
        dev->sector_size = 512;

    dev->capacity = ident.lba_capacity * dev->sector_size;

    return true;
}

static void registrant(ata_device_t* ata_device)
{
    if(!identify(ata_device))
        return;
    darray_append(ata_devices, *ata_device);
}

void ata_init()
{
    ata_devices = darray(ata_device_t, 0);
    ahci_init();
}

void ata_register_device(uint8_t bus, uint8_t dev, uint8_t fun)
{
    pci_device_t device = pci_get_device(bus, dev, fun);

    switch (device.subclass)
    {
    case PCI_ATA_SUB_SATA:
        ahci_register_device(&device, registrant);
        break;

    default:
        kernel_warn("Ignoring ATA device [subclass: %02X]", device.subclass);
        break;
    }
}

size_t ata_ndevices()
{
    return darray_length(ata_devices);
}

bool ata_send_cmd(uint64_t dev_id, ata_command_t* command, uint8_t* data, size_t transfer_len)
{
    if(dev_id >= darray_length(ata_devices))
        return false;
    ata_device_t* dev = &ata_devices[dev_id];
    
    return dev->send_ata_cmd(dev->internal_id, command, data, transfer_len);
}

bool ata_read(uint64_t dev_id, uint64_t lba, size_t nsectors, void* buffer)
{
    if(dev_id >= darray_length(ata_devices))
        return false;
    ata_device_t* dev = &ata_devices[dev_id];

    if(nsectors == 0)
        return true;

    ata_command_t cmd;
    memset(&cmd, 0, sizeof(ata_command_t));
    cmd.command = dev->lba48 ? ATA_CMD_READEXTDMA : ATA_CMD_READDMA;
    cmd.lba = lba;
    cmd.n_sectors = nsectors;
    cmd.write = false;
    cmd.lba28 = !dev->lba48;

    return ata_send_cmd(dev_id, &cmd, buffer, dev->sector_size * nsectors);
}

bool ata_write(uint64_t dev_id, uint64_t lba, size_t nsectors, void* buffer)
{
    if(dev_id >= darray_length(ata_devices))
        return false;
    ata_device_t* dev = &ata_devices[dev_id];

    if(nsectors == 0)
        return true;

    ata_command_t cmd;
    memset(&cmd, 0, sizeof(ata_command_t));
    cmd.command = dev->lba48 ? ATA_CMD_WRITEEXTDMA : ATA_CMD_WRITEDMA;
    cmd.lba = lba;
    cmd.n_sectors = nsectors;
    cmd.write = true;
    cmd.lba28 = !dev->lba48;

    return ata_send_cmd(dev_id, &cmd, buffer, dev->sector_size * nsectors);
}

size_t ata_sector_size(uint64_t dev_id)
{
    if(dev_id >= darray_length(ata_devices))
        return 0;
    ata_device_t* dev = &ata_devices[dev_id];

    return dev->sector_size;
}

size_t ata_capacity(uint64_t dev_id)
{
    if(dev_id >= darray_length(ata_devices))
        return 0;
    ata_device_t* dev = &ata_devices[dev_id];

    return dev->capacity;
}
