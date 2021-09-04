#pragma once

#include <stdint.h>

typedef struct rsdp_descriptor
{
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__ ((packed)) rsdp_descriptor_t;

typedef struct rsdp_descriptor_20
{
    rsdp_descriptor_t first_part;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) rsdp_descriptor_20_t;

typedef struct sdt_header
{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_iD;
  uint32_t creator_revision;
} __attribute__ ((packed)) sdt_header_t;

typedef struct rsdt_decriptor
{
    sdt_header_t header;
    uint32_t addresses[];
} __attribute__ ((packed)) rsdt_decriptor_t;

void acpi_init();
const rsdp_descriptor_t* acpi_rsdp();
const rsdt_decriptor_t* acpi_rsdt();