#include "dbg.h"
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>

/**
 * Send a character
 */
void dbg_uart_putc(unsigned int c)
{
    asm volatile (
        "mov ecx, 10000"    "\n"
        "mov dx, 0x3fd"     "\n"
        "one: inb al, dx"   "\n"
        "pause"             "\n"
        "cmp al, 0xff"      "\n"
        "je two"            "\n"
        "dec ecx"           "\n"
        "jz two"            "\n"
        "and al, 0x20"      "\n"
        "jz one"            "\n"
        "sub dl, 5"         "\n"
        "mov al, bl"        "\n"
        "outb dx, al"       "\n"
        "two:"
        :: "b"(c):"rax","rcx","rdx"
    );
}


void on_debug(const interrupt_context_t* context)
{
    for(;;);
}

void dbg_init()
{
    for(int i = 0; i < 10; i++)
    {
        dbg_uart_putc('e');
    }
    isr_set(EXCEPTION_DEBUG, on_debug);
}