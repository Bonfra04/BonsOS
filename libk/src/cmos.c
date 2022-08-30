#include <cmos.h>
#include <io/ports.h>

#define SELECT_PORT 0x70
#define REGISTER_PORT 0x71

uint8_t cmos_read_byte(uint8_t reg)
{
    outportb(SELECT_PORT, reg);
    port_wait();
    return inportb(REGISTER_PORT);
}

void cmos_write_byte(uint8_t reg, uint8_t value)
{
    outportb(SELECT_PORT, reg);
    port_wait();
    outportb(REGISTER_PORT, value);
}