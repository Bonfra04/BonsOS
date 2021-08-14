#include <interrupt/pic.h>
#include <x86/ports.h>

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

static void remap_pic()
{
    // int PICs
    outportb(PIC_CMD_MASTER, ICW1_INIT | ICW1_ICW4);
    port_wait();
    outportb(PIC_CMD_SLAVE, ICW1_INIT | ICW1_ICW4);
    port_wait();

    // set PICs offsets
    outportb(PIC_DATA_MASTER, 0x20);
    port_wait();
    outportb(PIC_DATA_SLAVE, 0x28);
    port_wait();

    // set PICs correspondings
    outportb(PIC_DATA_MASTER, 4);
    port_wait();
    outportb(PIC_DATA_SLAVE, 2);
    port_wait();

    // set PICs in 8086 mode
    outportb(PIC_DATA_MASTER, ICW4_8086);
    port_wait();
    outportb(PIC_DATA_SLAVE, ICW4_8086);
    port_wait();
}

void pic_init()
{
    remap_pic();
    pic_disable();
}

void pic_disable()
{
    // diable all ISRs
    outportb(PIC_DATA_MASTER, 0xFF);
    port_wait();
    outportb(PIC_DATA_SLAVE, 0xFF);
    port_wait();
}