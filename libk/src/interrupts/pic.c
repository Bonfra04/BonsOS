#include <interrupts/pic.h>
#include <io/ports.h>

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

#define PIC_CMD_MASTER  0x20
#define PIC_CMD_SLAVE   0xA0
#define PIC_DATA_MASTER 0x21
#define PIC_DATA_SLAVE  0xA1

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

    // set PICs corresponding
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

void pic_disable()
{
    remap_pic();

    outportb(PIC_DATA_MASTER, 0xFF);
    port_wait();
    outportb(PIC_DATA_SLAVE, 0xFF);
    port_wait();
}