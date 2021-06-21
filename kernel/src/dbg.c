#include "dbg.h"
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

static void dbg_putc(unsigned int c)
{
    asm volatile (
        "mov ecx, 10000"            "\n"
        "mov dx, 0x3fd"             "\n"
        "dbg_putc_one: inb al, dx"  "\n"
        "pause"                     "\n"
        "cmp al, 0xff"              "\n"
        "je dbg_putc_two"           "\n"
        "dec ecx"                   "\n"
        "jz dbg_putc_two"           "\n"
        "and al, 0x20"              "\n"
        "jz dbg_putc_one"           "\n"
        "mov al, bl"                "\n"
        "sub dl, 5"                 "\n"
        "outb dx, al"               "\n"
        "dbg_putc_two:"             "\n"
        :: "b"(c):"rax","rcx","rdx"
    );
}

static char dbg_getc() {
    char r;
    asm volatile(
        "mov dx, 0x3fd"         "\n"
        "dbg_getc_one: pause"   "\n"
        "inb al, dx"            "\n"
        "and al, 1"             "\n"
        "jz dbg_getc_one"       "\n"
        "sub dl, 5"             "\n"
        "inb al, dx"            "\n"
        : "=a"(r)::"rdx"
    );
    return r == '\r' ? '\n' : r;
}

static void dbg_printf(char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buff[512], *s = (char*)&buff;
    vsprintf(buff, format, args);
    va_end(args);
    while(*s) {
        if(*s == '\n')
            dbg_putc('\r');
        dbg_putc(*s++);
    }
}

static char* dbg_gets(char* str)
{
    char* ptr = str;
    do {
        *ptr = dbg_getc();
    } while(*ptr++ != '\n');
    *--ptr = '\0';
    return str;
}

void on_debug(const interrupt_context_t* context)
{
    char cmd[512];
    while(true)
    {
        dbg_printf("BonsOS@dbg > ");
        dbg_gets(cmd);

        if(cmd[0] == 0)
        {
            dbg_printf("Use `help` to get a list of commands\n");
            continue;
        }

        if(cmd[0] == 'c') // continue
            break;
    }
}

void dbg_init()
{
    dbg_printf("BonsOS debugger initialized\n");
    isr_set(EXCEPTION_BREAKPOINT, on_debug);
}