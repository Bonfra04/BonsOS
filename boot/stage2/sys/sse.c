#include "sse.h"
#include "cpuid.h"

bool fx_supported()
{
    cpuid_out_t cpuid_out;
    cpuid(1, &cpuid_out);
    return (cpuid_out.edx & (1 << 24)) != 0;
}

bool sse_supported()
{
    cpuid_out_t cpuid_out;
    cpuid(1, &cpuid_out);
    return (cpuid_out.edx & (1 << 25)) != 0;
}