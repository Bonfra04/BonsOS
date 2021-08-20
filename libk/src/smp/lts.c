#include <smp/lts.h>

#include <x86/cpu.h>
#include <stdint.h>
#include <string.h>
#include <interrupt/apic.h>

#define GS_BASE 0xC0000101

static cpu_lts_t cpu_lts[256];

#define current_lts (cpu_lts[lapic_get_id()])

void lts_init(const cpu_lts_t* new_cpu_lts)
{
    lts_set(new_cpu_lts);
    memset(&current_lts, 0, sizeof(cpu_lts_t));
    wrmsr(GS_BASE, &current_lts);
}

void lts_set(const cpu_lts_t* new_cpu_lts)
{
    memcpy(&current_lts, new_cpu_lts, sizeof(cpu_lts_t));
    current_lts.cpu_id = lapic_get_id();
}