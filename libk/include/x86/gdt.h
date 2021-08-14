#pragma once

#define SELECTOR_KERNEL_CODE 0x08
#define SELECTOR_KERNEL_DATA 0x10

#define SELECTOR_USER_CODE 0x20
#define SELECTOR_USER_DATA 0x18

#define SELECTOR_TSS 0x28

/**
 * @brief Initialize the gdt
 */
void gdt_init();

void gdt_install();

/**
 * @brief Sets the stack pointer in the tss
 */
void tss_set_kstack(void* stack_top);

/**
 * @brief Gets the current tss stack ponter
 */
void* __tss_get_kstack(); //inline version
void* tss_get_kstack();