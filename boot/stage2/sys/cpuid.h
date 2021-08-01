#pragma once

#include "../lib/stdbool.h"
#include "../lib/stdint.h"

typedef struct cpuid_out
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_out_t;

bool cpuid_unsupported();
void cpuid(uint32_t code, cpuid_out_t* regs);

char* cpuid_vendor(char vendor[13]);

bool cpuid_longmode_supported();
