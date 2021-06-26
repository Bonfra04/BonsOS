#include "dbg.h"
#include <interrupt/interrupt.h>
#include <interrupt/exception.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <uart/uart.h>
#include <stdlib.h>
#include <string.h>

static bool tracing;
static bool initialized;

static void dump_memory(uint64_t from, size_t length)
{
    uint64_t* mem = (uint64_t*)from;

    for(size_t i = 0; i < length; i++)
    {
        if(i != 0 && i % 5 == 0)
            uart_printf("\n");
        uart_printf("0x%X ", mem[i]);
    }

    if(length - 1 % 5 != 0)
        uart_printf("\n");
}

static void dump_registers(const interrupt_context_t* context)
{
    uart_printf("CS:RIP: %X:%llX   SS:RSP: %X:%llX\n", context->cs, context->retaddr, context->ss, context->rsp);

    uart_printf(
        "RAX: %llX    RSI: %llX    R11: %llX\n"
        "RBX: %llX    RDI: %llX    R12: %llX\n"
        "RCX: %llX     R8: %llX    R13: %llX\n"
        "RDX: %llX     R9: %llX    R14: %llX\n"
        "RBP: %llX    R10: %llX    R15: %llX\n",
        context->regs.rax, context->regs.rsi, context->regs.r11,
        context->regs.rbx, context->regs.rdi, context->regs.r12,
        context->regs.rcx, context->regs.r8, context->regs.r13,
        context->regs.rdx, context->regs.r9, context->regs.r14,
        context->regs.rbp, context->regs.r10, context->regs.r15
    );
}

static char* help = "BonsOS integrated debugger"
"\n""Commands list:"
"\n""   h : show this message"
"\n""   c : continue exectution"
"\n""   s : single setp instructions"
"\n""   n : execute next instruction (ignore function calls)"
"\n""   r : dump registers"
"\n""   x : dump memory area"
"\n";

static void on_debug(const interrupt_context_t* context)
{
    if(!initialized)
        return;

    char cmd[512];
    static bool last_was_call;
    static uint16_t call_byte;
    static void* call_ret;

    if(tracing)
    {
        if(last_was_call)
        {
            last_was_call = false;
            ((interrupt_context_t*)context)->rflags &= ~(1 << 8); // trap flag
            call_ret = (void*)*(uint64_t*)context->rsp;
            uint16_t* instruction = (uint16_t*)call_ret;
            call_byte = *instruction;
            *instruction = 0x01CD; // int 1
            return;
        }
        if(context->retaddr - (uint64_t)call_ret == 2)
        {
            ((interrupt_context_t*)context)->retaddr = (uint64_t)call_ret;
            *(uint16_t*)call_ret = call_byte;
            call_ret = 0;
        }
        uart_printf("CS:RIP: %X:%llX   SS:RSP: %X:%llX\n", context->cs, context->retaddr, context->ss, context->rsp);
    }
    else
    {
        last_was_call = false;
        call_ret = 0;
    }

    while(true)
    {
        memset(cmd, 0, 512);
        uart_printf("BonsOS@dbg > ");
        uart_gets(cmd);

        if(cmd[0] == 0)
        {
            uart_printf("Use `h` to get a list of commands\n");
        }
        else if(cmd[0] == 'h') // help
        {
            uart_printf(help);
        }
        else if(cmd[0] == 'c') // continue
        {
            ((interrupt_context_t*)context)->rflags &= ~(1 << 8); // trap flag
            tracing = false;
            break;
        }
        else if(cmd[0] == 's') // single setp
        {
            ((interrupt_context_t*)context)->rflags |= (1 << 8); // trap flag
            tracing = true;
            break;
        }
        else if(cmd[0] == 'n') // next instruction
        {
            uint8_t* opcode = (uint8_t*)context->retaddr;
            if(*opcode == 0xFF) // call opcode
                last_was_call = true;

            ((interrupt_context_t*)context)->rflags |= (1 << 8); // trap flag
            tracing = true;
            break;
        }
        else if(cmd[0] == 'r') // dump registers
            dump_registers(context);
        else if(cmd[0] == 'x')  // dump memory
        {
            char* ptr = cmd + 2;
            uint64_t from = strtoull(ptr, &ptr, ptr[0] == '0' ? ptr[1] == 'x' ? 16 : 8 : 10);
            uint64_t len = strtoull(ptr, &ptr, ptr[0] == '0' ? ptr[1] == 'x' ? 16 : 8 : 10);

            if(from <= 0) {
                uart_printf("Invalid syntax. Usage: `x <from> [<len>]`\n");
                continue;
            }
            if(len <= 0)
                len = 1;

            dump_memory(from, len);
        }

    }
}

void dbg_init()
{
    initialized = false;
    tracing = false;
    isr_set(EXCEPTION_BREAKPOINT, on_debug);
    isr_set(EXCEPTION_DEBUG, on_debug);
    uart_init();
    uart_printf("BonsOS debugger initialized\n");
    initialized = true;
    return true;
}