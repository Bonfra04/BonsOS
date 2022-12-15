#pragma once

#include <stdint.h>

#define HBA_GHC_AE      (1 << 31)
#define HBA_GHC_HR      (1 << 0)

#define HBA_CAP_NCS     (0b11111)

#define HBA_CAP2_BOH    (1 << 0)

#define HBA_BOHC_OOS    (1 << 1)
#define HBA_BOHC_BOS    (1 << 0)

#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

#define HBA_PxTFD_STS_DRQ   (1 << 3)
#define HBA_PxTFD_STS_BSY   (1 << 7)

#define HBA_PxSSTS_DET      (0b1111)

#define HBA_PxSI_TFES       (1 << 30)

#define HBA_PxSERR_DIAG_X (1 << 26)

#define HBA_PxSCTL_DET_RESET    0x1

#define HBA_PxSSTS_DET_DEV_PHY  0b011

#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35
#define ATA_CMD_IDENTIFY        0xEC
#define ATA_CMD_SEND_PACKET     0xA0

#define AHCI_SIG_ATA    0x00000101  // SATA drive
#define AHCI_SIG_ATAPI  0xEB140101  // SATAPI drive

typedef enum fis_type
{
    FIS_TYPE_REG_H2D    = 0x27,    // Register FIS - host to device
    FIS_TYPE_REG_D2H    = 0x34,    // Register FIS - device to host
    FIS_TYPE_DMA_ACt    = 0x39,    // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP  = 0x41,    // DMA setup FIS - bidirectional
    FIS_TYPE_DATA       = 0x46,    // Data FIS - bidirectional
    FIS_TYPE_BIST       = 0x58,    // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP  = 0x5F,    // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS   = 0xA1,    // Set device bits FIS - device to host
} fis_type_t;

typedef struct hba_cmd_header
{
    // DW0
    uint8_t  cfl : 5;    // Command FIS length in DWORDS, 2 ~ 16
    uint8_t  a : 1;      // ATAPI
    uint8_t  write : 1;      // Write, 1: H2D, 0: D2H
    uint8_t  p : 1;      // Prefetchable

    uint8_t  r : 1;      // Reset
    uint8_t  b : 1;      // BIST
    uint8_t  c : 1;      // Clear busy upon R_OK
    uint8_t  reserved0 : 1; // Reserved
    uint8_t  pmp : 4;    // Port multiplier port

    uint16_t prdtl;     // Physical region descriptor table length in entries

    // DW1
    volatile int32_t prdbc; // Physical region descriptor byte count transferred

    // DW2, 3
    uint32_t ctba;      // Command table descriptor base address
    uint32_t ctbau;     // Command table descriptor base address upper 32 bits

    // DW4 - 7
    uint32_t reserved1[4]; // Reserved
} __attribute__ ((packed)) hba_cmd_header_t;

typedef struct fis_reg_h2d
{
    // DWORD 0
    uint8_t  fis_type;   // FIS_TYPE_REG_H2D

    uint8_t  pmport : 4; // Port multiplier
    uint8_t  reserved0 : 3;// Reserved
    uint8_t  c : 1;      // 1: Command, 0: Control

    uint8_t  command;    // Command register
    uint8_t  featurel;   // Feature register, 7:0

    // DWORD 1
    uint8_t  lba0;       // LBA low register, 7:0
    uint8_t  lba1;       // LBA mid register, 15:8
    uint8_t  lba2;       // LBA high register, 23:16
    uint8_t  device;      // Device register

    // DWORD 2
    uint8_t  lba3;       // LBA register, 31:24
    uint8_t  lba4;       // LBA register, 39:32
    uint8_t  lba5;       // LBA register, 47:40
    uint8_t  featureh;   // Feature register, 15:8

    // DWORD 3
    uint8_t  countl;     // Count register, 7:0
    uint8_t  counth;     // Count register, 15:8
    uint8_t  icc;        // Isochronous command completion
    uint8_t  control;    // Control register

    // DWORD 4
    uint8_t  reserved1[4];// Reserved
} __attribute__ ((packed)) fis_reg_h2d_t;

typedef struct hba_prdt_entry
{
    uint32_t dba;       // Data base address
    uint32_t dbau;      // Data base address upper 32 bits
    uint32_t rsv0;      // Reserved

    // DW3
    uint32_t dbc : 22;  // Byte count, 4M max
    uint32_t rsv1 : 9;  // Reserved
    uint32_t i : 1;     // Interrupt on completion
} __attribute__ ((packed)) hba_prdt_entry_t;

typedef struct hba_cmd_table
{
    uint8_t  cfis[64];   // Command FIS
    uint8_t  acmd[16];   // ATAPI command, 12 or 16 bytes
    uint8_t  rsv[48];    // Reserved

    hba_prdt_entry_t prdt_entry[];  // Physical region descriptor table entries, 0 ~ 65535
} __attribute__ ((packed)) hba_cmd_table_t;

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
    uint32_t dev_slp;   // 0x44, device sleep
    uint8_t reserved[40];// 0x44 ~ 0x6F, Reserved
    uint8_t vendor[16]; // 0x70 ~ 0x7F, vendor specific
} __attribute__ ((packed)) hba_port_t;

typedef struct hba_mem
{
    uint32_t cap;       // 0x00, Host capability
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

    uint8_t  reserved[0x74];
    uint8_t  vendor[0x60];

    volatile hba_port_t ports[];    // 1 ~ 32
} __attribute__ ((packed)) hba_mem_t;

typedef struct ahci_command
{
    uint8_t index; 
    volatile hba_cmd_table_t* cmd_table;
} ahci_command_t;

typedef struct ahci_device
{
    size_t ncmd;
    uint32_t allocated_cmd;
    volatile hba_cmd_header_t* cmd_header;
    volatile hba_port_t* port;
} ahci_device_t;
