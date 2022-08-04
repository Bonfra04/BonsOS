#pragma once

#include <containers/darray.h>
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

/**
 * @brief locates and reads the acpi tables
 */
void acpi_init();

/**
 * @brief returns the address of the requested acpi table entry
 * @param[in] signature the signature of the table entry to find
 * @return the address of the requested acpi table entry
 */
void* acpi_find_entry(const char signature[4]);
