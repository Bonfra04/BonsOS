#pragma once

#define SELECTOR_KERNEL_CODE 0x08
#define SELECTOR_KERNEL_DATA 0x10

#define SELECTOR_USER_CODE 0x20
#define SELECTOR_USER_DATA 0x18

#define SELECTOR_TSS 0x28

/**
 * @brief installs the gdt for the calling core
 */
void gdt_install();