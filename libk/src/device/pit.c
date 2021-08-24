#include <device/pit.h>
#include <stddef.h>
#include <stdbool.h>

#define PIT_CHANNEL0_DATA   0x40
#define PIT_CHANNEL1_DATA   0x41
#define PIT_CHANNEL2_DATA   0x42
#define PIT_COMMAND_REG     0x43

#define PIT_COMMAND_CHANNEL(x)          (x << 6)
#define PIT_COMMAND_ACCESS_LATCH_COUNT  (0)
#define PIT_COMMAND_ACCESS_LOBYTE_ONLY  (1 << 4)
#define PIT_COMMAND_ACCESS_HIBYTE_ONLY  (1 << 5)
#define PIT_COMMAND_ACCESS_LOHIBYTE     (PIT_COMMAND_ACCESS_LOBYTE_ONLY | PIT_COMMAND_ACCESS_HIBYTE_ONLY)
#define PIT_COMMAND_MODE(x)             (x << 1)
#define PIT_COMMAND_BINARY_MODE         (0 << 0)
#define PIT_COMMAND_BCD_MODE            (1 << 0)

#define PIT_READBACK_MUST_ONE           (1 << 7) | (1 << 6)
#define PIT_READBACK_DONT_LATCH_COUNT   (1 << 5)
#define PIT_READBACK_DONT_LATCH_STATUS  (1 << 4)
#define PIT_READBACK_CHANNEL2           (1 << 3)
#define PIT_READBACK_CHANNEL1           (1 << 2)
#define PIT_READBACK_CHANNEL0           (1 << 1)

#define PIT_STATUS_OUTPUT_HIGH (1 << 7)
#define PIT_FREQUENCY 1193180

static void send_readback(uint8_t channels, bool count, bool status)
{
    uint8_t command = 0;
    command |= PIT_READBACK_MUST_ONE;
    if(!count)
        command |= PIT_READBACK_DONT_LATCH_COUNT;
    if(!status)
        command |= PIT_READBACK_DONT_LATCH_STATUS;

    command |= (channels & 0x3);

    outportb(PIT_COMMAND_REG, command);
}

void pit_prepare_one_shot(uint32_t frequency)
{
    uint32_t divisor = PIT_FREQUENCY / frequency;

    uint8_t command = PIT_COMMAND_CHANNEL(0) | PIT_COMMAND_ACCESS_LOHIBYTE | PIT_COMMAND_MODE(0) | PIT_COMMAND_BINARY_MODE;

    outportb(PIT_COMMAND_REG, command);

    outportb(PIT_CHANNEL0_DATA, divisor & 0xFF);
    outportb(PIT_CHANNEL0_DATA, divisor >> 8);
}

void pit_wait_one_shot()
{
    uint8_t status = 0;
    do
    {
        send_readback(PIT_READBACK_CHANNEL0, false, true);
        status = inportb(PIT_CHANNEL0_DATA);
    } while(!(status & PIT_STATUS_OUTPUT_HIGH));
}
