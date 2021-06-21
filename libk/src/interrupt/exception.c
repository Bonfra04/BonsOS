#include <interrupt/exception.h>
#include <interrupt/interrupt.h>
#include <device/tty.h>
#include <x86/cpu.h>
#include <dump.h>
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

static void dump_context(const interrupt_context_t* context)
{
    printf("INT: 0x%llX   Error: 0x%llX\n\n", context->interrupt, context->error);
    printf("CS:RIP: %x:%llX   SS:RSP: %x:%llX\n\n", context->cs, context->retaddr, context->ss, context->rsp);

    char buf[640];

    dump_registers(buf, sizeof(buf), &context->regs);
    printf("%s\n", buf);

    dump_cpuflags(buf, sizeof(buf), context->rflags);
    printf("%s\n", buf);

    uint64_t cr2;
    asm("mov %0, cr2" : "=r"(cr2));
    printf("CR2: %llX\n", cr2);

    // cannot deduce stack base so causing page fault
    /*
        printf("\nStack:\n");
        void* stack = (void*)context->rsp;
        dump_memory(buf, sizeof(buf), stack - 8 * 16, 8 * 16, DUMPSTYLE_ADDR);
        printf(buf);
    */

    printf("\nCode:\n");
    void* rip = (void*)context->retaddr;
    dump_memory(buf, sizeof(buf), rip, 8 * 16, DUMPSTYLE_ADDR);
    printf(buf);
}

static void hang()
{
    for (;;)
    {
        disable_interrupts();
        halt();
    }
}

static void isr_fatal(const interrupt_context_t* context)
{
    int i = context->interrupt;

    const char* exstr = i < ((int)(sizeof(exceptionstr) / sizeof(exceptionstr[0])))
                        ? exceptionstr[i] : "Unknown exception.";

    tty_set_textcolor(0xFFFFFFFF, 0xFFFF0000); // white, red
    tty_clear();
    printf("%s\n\n", exstr);

    dump_context(context);

    hang();
}

void exceptions_init()
{
    for (int i = 0; i < 32; i++)
        isr_set(i, isr_fatal); // fatal for now. temporary.
    isr_set(0xFF, isr_fatal);
}