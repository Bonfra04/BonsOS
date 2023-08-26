#pragma once

#include <drivers/storage/scsi.h>

#define SCSI_COMMAND_INQUIRY 0x12

typedef struct scsi_command_inquiry
{
    uint8_t command;
    uint8_t evpd;
    uint8_t page_code;
    uint16_t allocation_length;
    uint8_t control;
} __attribute__((packed)) scsi_command_inquiry_t;

typedef struct scsi_inquiry
{
    struct
    {
        uint8_t peripheral_device_type : 5;
        uint8_t peripheral_qualifier : 3;
    };
    struct
    {
        uint8_t reserved0 : 7;
        uint8_t rmb : 1;
    };
    uint8_t version;
    struct
    {
        uint8_t response_data_format : 4;
        uint8_t hisup : 1;
        uint8_t normaca : 1;
        uint8_t obsolete : 2;
    };
    uint8_t additional_length;
    struct
    {
        uint8_t protect : 1;
        uint8_t reserved1 : 2;
        uint8_t third_party_copy : 1;
        uint8_t tpgs : 2;
        uint8_t acc : 1;
        uint8_t sccs : 1;
    };
    struct
    {
        uint8_t obsolete0 : 4;
        uint8_t multip : 1;
        uint8_t vs0 : 1;
        uint8_t encserv : 1;
        uint8_t obsolete1 : 1;
    };
    struct
    {
        uint8_t vs1 : 1;
        uint8_t cmdque : 1;
        uint8_t obsolete2 : 6;
    };
    uint8_t vendor_id[8];
    uint8_t product_id[16];
    uint8_t product_revision[4];
} __attribute__((packed)) scsi_inquiry_t;

#define SCSI_COMMAND_READ_CAPACITY 0x25

typedef struct scsi_command_read_capacity
{
    uint8_t command;
    uint8_t reserved0;
    uint32_t lba;
    uint16_t reserved1;
    uint8_t reserved2;
    uint8_t control;
} __attribute__((packed)) scsi_command_read_capacity_t;

typedef struct scsi_read_capacity
{
    uint32_t lba;
    uint32_t block_size;
} __attribute__((packed)) scsi_read_capacity_t;

#define SCSI_COMMAND_READ12 0xA8

typedef struct scsi_command_read12
{
    uint8_t command;
    struct
    {
        uint8_t obsolete0 : 2;
        uint8_t rarc : 1;
        uint8_t fua : 1;
        uint8_t dpo : 1;
        uint8_t rdprotect : 3;
    };
    uint32_t lba;
    uint32_t length;
    struct
    {
        uint8_t group_number : 5;
        uint8_t reserved : 2;
        uint8_t restricted : 1;
    };
    uint8_t control;
} __attribute__((packed)) scsi_command_read12_t;

#define SCSI_COMMAND_READ10 0x28

typedef struct scsi_command_read10
{
    uint8_t command;
    struct
    {
        uint8_t obsolete0 : 2;
        uint8_t rarc : 1;
        uint8_t fua : 1;
        uint8_t dpo : 1;
        uint8_t rdprotect : 3;
    };
    uint32_t lba;
    struct
    {
        uint8_t group_number : 5;
        uint8_t reserved : 3;
    };
    uint16_t length;
    uint8_t control;
} __attribute__((packed)) scsi_command_read10_t;

#define SCSI_COMMAND_WRITE12 0xAA

typedef struct scsi_command_write12
{
    uint8_t command;
    struct
    {
        uint8_t obsolete0 : 2;
        uint8_t reserved0 : 1;
        uint8_t fua : 1;
        uint8_t dpo : 1;
        uint8_t wrprotect : 3;
    };
    uint32_t lba;
    uint32_t length;
    struct
    {
        uint8_t group_number : 5;
        uint8_t reserved1 : 2;
        uint8_t restricted : 1;
    };
    uint8_t control;
} __attribute__((packed)) scsi_command_write12_t;

#define SCSI_COMMAND_WRITE10 0x2A

typedef struct scsi_command_write10
{
    uint8_t command;
    struct
    {
        uint8_t obsolete0 : 2;
        uint8_t reserved0 : 1;
        uint8_t fua : 1;
        uint8_t dpo : 1;
        uint8_t wrprotect : 3;
    };
    uint32_t lba;
    struct
    {
        uint8_t group_number : 5;
        uint8_t reserved1 : 3;
    };
    uint16_t length;
    uint8_t control;
} __attribute__((packed)) scsi_command_write10_t;

typedef struct scsi_device
{
    scsi_driver_t driver;
    size_t capacity;
    size_t sector_size;
} scsi_device_t;
