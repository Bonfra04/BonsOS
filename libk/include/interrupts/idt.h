#pragma once

/**
 * @brief initializes the idt populating the table with the default entires
 */
void idt_init();

/**
 * @brief installs the idt for the calling core
 */
void idt_install();
