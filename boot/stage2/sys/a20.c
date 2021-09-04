#include "a20.h"
#include "../lib/stdint.h"
#include "realmode.h"
#include "ports.h"

static bool a20_enabled()
{
    register uint32_t* a = (uint32_t*)0x112345; // odd megabyte address
    register uint32_t* b = (uint32_t*)0x012345; // even megabyte address
    
    // set addresses to different values
    *a = 0x1A20;
    *b = 0x0A20;

    // if both values are equal a20 is disables cause the 20th bit has ben discarded
    return *a != *b;
}

bool a20_enable()
{
    if(a20_enabled())
        return true;

    // try with BIOS
    rm_regs_t regs = { 0 };
    regs.eax = 0x2401;
    rm_int(0x15, &regs, &regs);

    if(a20_enabled())
        return true;

    extern void a20_kb();
    a20_kb();

    if(a20_enabled())
        return true;

    return false;
}