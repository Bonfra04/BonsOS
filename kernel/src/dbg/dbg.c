#include "dbg.h"
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <uart/uart.h>

static void on_debug(const interrupt_context_t* context)
{
    char cmd[512];
    while(true)
    {
        uart_printf("BonsOS@dbg > ");
        uart_gets(cmd);

        if(cmd[0] == 0)
        {
            uart_printf("Use `help` to get a list of commands\n");
            continue;
        }

        if(cmd[0] == 'c') // continue
            break;
    }
}

void dbg_init()
{
    uart_init();
    isr_set(EXCEPTION_BREAKPOINT, on_debug);
    uart_printf("BonsOS debugger initialized\n");
}