#include <x86/ports.h>

void port_delay(uint64_t cycles)
{
    while(cycles--)
        inportb(0x80);
}