#pragma once

#include "../lib/stdint.h"

typedef struct rm_regs
{
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
} __attribute__((packed)) __attribute__ ((aligned (16))) rm_regs_t;

/**
 * @brief execute a bios interrupt
 * @param[in] interrupt interrupt number
 * @param[in] regs_in register to pass to the interrupt
 * @param[out] regs_out register to receive the result of the interrupt
 */
void rm_int(uint8_t interrupt, rm_regs_t* regs_in, rm_regs_t* regs_out);
