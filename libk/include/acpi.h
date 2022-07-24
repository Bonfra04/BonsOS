#pragma once

#include <containers/darray.h>
#include <stdint.h>

typedef struct lapic_entry
{
    uint8_t acpi_processor_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) lapic_entry_t;

typedef struct ioapic_entry
{
    uint8_t ioapic_id;
    uint8_t reserved;
    uint32_t ioapic_address;
    uint32_t gsi_base;
} __attribute__((packed)) ioapic_entry_t;

typedef struct ioapic_int_src_ovr_entry
{
    uint8_t bus_source;
    uint8_t irq_source;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) ioapic_int_src_ovr_entry_t;

typedef struct madt_data
{
    void* lapic_address;
    lapic_entry_t* lapic_entries;
    ioapic_entry_t* ioapic_entries;
    ioapic_int_src_ovr_entry_t* ioapic_int_src_ovr_entries;
} madt_data_t;

/**
 * @brief locates and reads the acpi tables
 */
void acpi_init();

/**
 * @brief creates a structure holding all madt relevant data
 * @return structure holding all madt relevant data
 */
madt_data_t acpi_get_madt();
