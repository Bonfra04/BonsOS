#include <device/mouse.h>
#include <interrupt/interrupt.h>
#include <x86/ports.h>

#define IRQ_MOUSE       0x0C
#define TRAP_IRQ_MOUSE  0x2C

#define MOUSE_CMD       0x60
#define MOUSE_STATUS    0x64

#define PS2_LEFT_BTN    0xb00000001
#define PS2_MIDDLE_BTN  0xb00000010
#define PS2_RIGHT_BTN   0xb00000100

#define PS2_X_SIGN_BIT 0b00010000
#define PS2_Y_SIGN_BIT 0b00100000

static uint64_t bound_x, bound_y;
static volatile uint64_t mouse_x, mouse_y;

static bool btnStates[0x3];
static uint8_t packet[3];
static uint8_t cycle;

static void process_packet()
{
    bool xNeg = (packet[0] & PS2_X_SIGN_BIT) != 0;
    bool yNeg = (packet[0] & PS2_Y_SIGN_BIT) != 0;

    int16_t dx = packet[1] - 256 * xNeg;
    int16_t dy = -(packet[2] - 256 * yNeg);

    if(mouse_y + dy < 0)
        mouse_y = 0;
    else if(mouse_y + dy >= bound_y)
        mouse_y = bound_y - 1;
    else
        mouse_y += dy;

    if(mouse_x + dx < 0)
        mouse_x = 0;
    else if(mouse_x + dx >= bound_x)
        mouse_x = bound_x - 1;
    else
        mouse_x += dx;

    btnStates[BTN_LEFT] = packet[0] & PS2_LEFT_BTN != 0;
    btnStates[BTN_MIDDLE] = packet[0] & PS2_MIDDLE_BTN != 0;
    btnStates[BTN_RIGHT] = packet[0] & PS2_RIGHT_BTN != 0;
}

static void isr_mouse(const interrupt_context_t* context)
{
    uint8_t data = inportb(MOUSE_CMD);
    if ((cycle == 0) && ((data & 0b00001000) == 0))
    {
        ISR_DONE();
        return;
    }

    packet[cycle] = data;
    if(cycle == 0 || cycle == 1)
        cycle++;
    else
    {
        cycle = 0;
        process_packet();
    }

    ISR_DONE();
}

static void mouse_wait_out()
{
    while((inportb(MOUSE_STATUS) & (1 << 1)))
        asm("pause");
}

static void mouse_wait_in()
{
    while(!(inportb(MOUSE_STATUS) & (1 << 0)))
        asm("pause");
}

static void mouse_send(uint8_t value)
{
    mouse_wait_out();
    outportb(MOUSE_STATUS, 0xD4);
    mouse_wait_out();
    outportb(MOUSE_CMD, value);
}

static void mouse_wait_ack()
{
    mouse_wait_in();
    while(inportb(MOUSE_CMD) != 0xFA)
        asm("pause");
}

void mouse_init(uint64_t boundX, uint64_t boundY)
{
    bound_x = boundX;
    bound_y = boundY;
    mouse_x = bound_x / 2;
    mouse_y = bound_y / 2;
    cycle = 0;

    outportb(MOUSE_STATUS, 0xA8); // enable ps2 aux

    mouse_wait_out();
    outportb(MOUSE_STATUS, 0x20); // get compacq status byte
    mouse_wait_in();
    uint8_t status = inportb(MOUSE_CMD);
    status |= (1 << 1); // enable irq12
    status &= ~(1 << 5); // disable muse clock
    mouse_wait_out();
    outportb(MOUSE_STATUS, 0x60); // set compacq status byte
    mouse_wait_out();
    outportb(MOUSE_CMD, status);

    mouse_send(0xF6); // set defaults
    mouse_wait_ack();
    mouse_send(0xF4); // ensable packets streaming
    mouse_wait_ack();

    isr_set(TRAP_IRQ_MOUSE, isr_mouse);
    irq_enable(IRQ_MOUSE);
}

inline bool mouse_is_down(uint8_t btn)
{
    return btnStates[btn];
}

inline uint64_t mouse_get_x()
{
    return mouse_x;
}

inline uint64_t mouse_get_y()
{
    return mouse_y;
}