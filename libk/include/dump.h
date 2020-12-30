#pragma once

#include <stddef.h>
#include <x86/cpu.h>

enum dumpstyle
{
    DUMPSTYLE_NOADDR = 0,        ///< No address or offset in memory dump.
    DUMPSTYLE_ADDR   = 1,        ///< Include full address in memory dump.
    DUMPSTYLE_OFFSET = 2,        ///< Include address offset in memory dump.
};

#ifdef __cplusplus
extern "C" {
#endif

int dump_registers(char* buf, size_t bufsize, const registers_t* regs);
int dump_cpuflags(char* buf, size_t bufsize, uint64_t rflags);
int dump_memory(char* buf, size_t bufsize, const void* mem, size_t memsize, enum dumpstyle style);

#ifdef __cplusplus
}
#endif