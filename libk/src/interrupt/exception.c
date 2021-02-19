#include <interrupt/exception.h>
#include <interrupt/interrupt.h>
#include <device/tty.h>
#include <x86/cpu.h>
#include <dump.h>

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
    tty_printf("INT: 0x%x   Error: 0x%x\n\n", context->interrupt, context->error);
    tty_printf("CS:RIP: %x:%x             SS:RSP: %x:%x\n\n", context->cs, context->retaddr, context->ss, context->rsp);

    char buf[640];

    dump_registers(buf, sizeof(buf), &context->regs);
    tty_printf("%s\n", buf);

    dump_cpuflags(buf, sizeof(buf), context->rflags);
    tty_printf("%s\n", buf);

    tty_printf("Stack:\n");
    void* stack = (void*)context->rsp;
    dump_memory(buf, sizeof(buf), stack, 8 * 16, DUMPSTYLE_ADDR);
    tty_printf(buf);
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

    tty_set_textcolor(TEXTCOLOR_WHITE, TEXTCOLOR_RED);
    tty_clear();
    tty_printf("%s\n\n", exstr);

    dump_context(context);

    hang();
}

static void isr_breakpoint(const interrupt_context_t *context)
{
    (void)context;
    tty_printf("Breakpoint hit.\n");
}

void exceptions_init()
{
    for (int i = 0; i < 32; i++)
        isr_set(i, isr_fatal); // fatal for now. temporary.
    isr_set(0xFF, isr_fatal);

    isr_set(EXCEPTION_BREAKPOINT, isr_breakpoint);
}