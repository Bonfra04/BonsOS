#include <smp/lts.h>

#include <x86/cpu.h>
#include <stdint.h>
#include <string.h>

#define GS_BASE 0xC0000101

static cpu_lts_t cpu_lts;

void lts_init()
{
    memset(&cpu_lts, 0, sizeof(cpu_lts_t));
    wrmsr(GS_BASE, &cpu_lts);
}

void lts_set(const cpu_lts_t* new_cpu_lts)
{
    memcpy(&cpu_lts, new_cpu_lts, sizeof(cpu_lts_t));
}