#pragma once

#include <stdint.h>

typedef struct cpu_lts
{
    uint8_t cpu_id;
    uint64_t kernel_stack;
} __attribute__ ((packed)) cpu_lts_t;

void lts_init(const cpu_lts_t* new_cpu_lts);
void lts_set(const cpu_lts_t* new_cpu_lts);
