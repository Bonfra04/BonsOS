#pragma once

#include <stdint.h>

typedef struct cpu_lts
{
    uint64_t kernel_stack;
    uint64_t user_stack;
} __attribute__ ((packed)) cpu_lts_t;

void lts_init();
void lts_set(const cpu_lts_t* new_cpu_lts);
