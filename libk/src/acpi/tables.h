#pragma once

#include <stdint.h>

typedef struct rsdt_header
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
} __attribute__ ((packed)) rsdt_header_t;

typedef enum madt_entry_type
{
    MADT_LAPIC = 0,
    MADT_IOAPIC = 1,
    MADT_INT_SRC_OVR = 2,
    MADT_NMI_SRC = 3,
    MADT_LAPIC_NMI = 4,
    MADT_LAPIC_OVR = 5,
    MADT_Lx2APIC = 9,
} madt_entry_type_t;

typedef struct madt_header
{
    rsdt_header_t header;
    uint32_t lapic_address;
    uint32_t flags;
} __attribute__((packed)) madt_header_t;

typedef struct madt_entry
{
    uint8_t entry_type;
    uint8_t record_lenght;
} __attribute__((packed)) madt_entry_t;

typedef struct lapic_addr_ovr_entry
{
    uint16_t reserved;
    uint64_t address;
} __attribute__((packed)) lapic_addr_ovr_entry_t;
