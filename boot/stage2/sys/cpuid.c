#include "cpuid.h"

char* cpuid_vendor(char vendor[13])
{
    cpuid_out_t cpuid_out;
    cpuid(0, &cpuid_out);

    vendor[0] = (uint8_t)(cpuid_out.ebx >> 0);
    vendor[1] = (uint8_t)(cpuid_out.ebx >> 8);
    vendor[2] = (uint8_t)(cpuid_out.ebx >> 16);
    vendor[3] = (uint8_t)(cpuid_out.ebx >> 24);
    
    vendor[4] = (uint8_t)(cpuid_out.edx >> 0);
    vendor[5] = (uint8_t)(cpuid_out.edx >> 8);
    vendor[6] = (uint8_t)(cpuid_out.edx >> 16);
    vendor[7] = (uint8_t)(cpuid_out.edx >> 24);

    vendor[8] = (uint8_t)(cpuid_out.ecx >> 0);
    vendor[9] = (uint8_t)(cpuid_out.ecx >> 8);
    vendor[10] = (uint8_t)(cpuid_out.ecx >> 16);
    vendor[11] = (uint8_t)(cpuid_out.ecx >> 24);

    vendor[12] = '\0';

    return vendor;
}

bool cpuid_longmode_supported()
{
    cpuid_out_t cpuid_out;

    // check if extended functions are supported
    cpuid(0x80000000, &cpuid_out);
    if(cpuid_out.eax <= 0x80000000)
        return false;
    
    cpuid(0x80000001, &cpuid_out);
    return (cpuid_out.edx & (1 << 29)) != 0;
}