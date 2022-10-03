#include <interrupts/exceptions.h>
#include <interrupts/isr_dispatcher.h>
#include <memory/paging.h>
#include <cpu.h>
#include <io/tty.h>
#include <io/uart.h>

#include <stdio.h>

static const char *exceptionstr[] =
{
    "#DE: Divide by zero exception",
    "#DB: Debug exception",
    "Non-maskable interrupt",
    "#BP: Breakpoint exception",
    "#OF: Overflow exception",
    "#BR: BOUND Range exceeded exception",
    "#UD: Invalid opcode exception",
    "#NM: Device not available exception",
    "#DF: Double fault exception",
    "Coprocessor segment overrun",
    "#TS: Invalid TSS exception",
    "#NP: Segment not present exception",
    "#SS: Stack fault exception",
    "#GP: General protection exception",
    "#PF: Page fault exception",
    "Unknown exception",
    "#MF: x87 FPU floating-point error",
    "#AC: Alignment check exception",
    "#MC: Machine-check exception",
    "#XM: SIMD floating-point exception",
    "#VE: Virtualization exception",
};

static void dump_context(const interrupt_context_t* context, char* str)
{
    int advance = 0;
    advance += sprintf(str + advance, "INT: %#llX   Error: %#llX\n\n", context->interrupt, context->error);
    advance += sprintf(str + advance, "CS:RIP: %llX:%llX   SS:RSP: %llX:%llX\n\n", context->cs, context->retaddr, context->ss, context->rsp);

    advance += sprintf(str + advance,
        "RAX: 0x%.16llX  RSI: 0x%.16llX  R11: 0x%.16llX\n"
        "RBX: 0x%.16llX  RDI: 0x%.16llX  R12: 0x%.16llX\n"
        "RCX: 0x%.16llX   R8: 0x%.16llX  R13: 0x%.16llX\n"
        "RDX: 0x%.16llX   R9: 0x%.16llX  R14: 0x%.16llX\n"
        "RBP: 0x%.16llX  R10: 0x%.16llX  R15: 0x%.16llX\n",
        context->registers.rax, context->registers.rsi, context->registers.r11,
        context->registers.rbx, context->registers.rdi, context->registers.r12,
        context->registers.rcx, context->registers.r8, context->registers.r13,
        context->registers.rdx, context->registers.r9, context->registers.r14,
        context->registers.rbp, context->registers.r10, context->registers.r15
    );

#define B(F)  ((context->rflags & F) ? 1 : 0)

    advance += sprintf(str + advance,
        "CF=%u  PF=%u  AF=%u  ZF=%u  SF=%u   "
        "TF=%u  IF=%u  DF=%u  OF=%u  IOPL=%u\n",
        B(CPU_EFLAGS_CARRY), B(CPU_EFLAGS_PARITY), B(CPU_EFLAGS_ADJUST),
        B(CPU_EFLAGS_ZERO), B(CPU_EFLAGS_SIGN), B(CPU_EFLAGS_TRAP),
        B(CPU_EFLAGS_INTERRUPT), B(CPU_EFLAGS_DIRECTION),
        B(CPU_EFLAGS_OVERFLOW), (context->rflags >> 12) & 3
    );

#undef B

    uint64_t cr2;
    asm("mov %0, cr2" : "=r"(cr2));
    advance += sprintf(str + advance, "CR2: 0x%llX\n", cr2);
}

static void isr_fatal(const interrupt_context_t* context)
{
    paging_enable(kernel_paging);

    int i = context->interrupt;

    const char* exstr = i < ((int)(sizeof(exceptionstr) / sizeof(exceptionstr[0])))
                        ? exceptionstr[i] : "Unknown exception.";

    char context_str[1024];
    dump_context(context, context_str);

    tty_set_textcolor(0xFFFFFFFF, 0xFFFF0000); // white, red
    tty_clear();

    tty_print(exstr, false);
    tty_print("\n\n", false);
    tty_print(context_str, false);

    uart_puts(exstr);
    uart_puts("\n\n");
    uart_puts(context_str);

    for(;;)
    {
        cli();
        hlt();
    }
}

void exceptions_init()
{
    for (uint8_t i = 0; i < 32; i++)
        isr_set(i, isr_fatal);
}
