#pragma once

#include <containers/linked_list.h>
#include <stdint.h>

typedef struct madt_data
{
    void* lapic_address;
    linked_list_t lapic_entries;
    linked_list_t ioapic_entries;
    linked_list_t ioapic_int_src_ovr_entries;
} madt_data_t;

/**
 * @brief locates and reads the acpi tables
 */
void acpi_init();

/**
 * @brief creates a structure holding all madt revelant data
 * @return structure holding all madt revelant data
 */
madt_data_t acpi_get_madt();

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
