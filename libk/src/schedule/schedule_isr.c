#include <interrupt/interrupt.h>
#include <string.h>
#include <schedule/scheduler.h>

typedef struct process_context
{
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;
    
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    
    uint64_t ss;
    uint64_t cs;
    
    uint64_t flags;
    
    uint64_t rip;
} __attribute__ ((packed)) process_context_t;

extern void set_thread_stack(size_t rsp);
extern thread_t* get_next_thread();

static process_context_t process_context;

static uint64_t tmp_stack[32];

void schedule_isr(const interrupt_context_t* context)
{
    process_context.rip = context->retaddr;
    process_context.rax = context->regs.rax;
    process_context.rbx = context->regs.rbx;
    process_context.rcx = context->regs.rcx;
    process_context.rdx = context->regs.rdx;
    process_context.rsi = context->regs.rsi;
    process_context.rdi = context->regs.rdi;
    process_context.rbp = context->regs.rbp;
    process_context.r8 = context->regs.r8;
    process_context.r9 = context->regs.r9;
    process_context.r10 = context->regs.r10;
    process_context.r11 = context->regs.r11;
    process_context.r12 = context->regs.r12;
    process_context.r13 = context->regs.r13;
    process_context.r14 = context->regs.r14;
    process_context.r15 = context->regs.r15;
    process_context.ds = context->ds;
    process_context.es = context->es;
    process_context.fs = context->fs;
    process_context.gs = context->gs;
    process_context.ss = context->ss;
    process_context.cs = context->cs;
    process_context.flags = context->rflags;

    // save thread context
    tmp_stack[0] = context->rsp;
    memcpy(tmp_stack[0] - sizeof(process_context_t), &process_context, sizeof(process_context_t));
    set_thread_stack(tmp_stack[0] - sizeof(process_context_t));

    // temporary stack
    asm volatile("mov rsp, %[addr]" : : [addr]"r"(tmp_stack + sizeof(tmp_stack)) : "memory");

    extern void restore_context();

    asm volatile("mov rsp, %[addr]" : : [addr]"r"(get_next_thread()->rsp) : "memory");
    asm volatile ("jmp restore_context");

    
    // asm volatile(
    //     "pop rax"       "\n"
    //     "mov ds, rax"   "\n"
    //     "pop rax"       "\n"
    //     "mov es, rax"   "\n"
    //     "pop rax"       "\n"
    //     "mov fs, rax"   "\n"
    //     "pop rax"       "\n"
    //     "mov gs, rax"   "\n"

    //     "pop rax"       "\n"
    //     "pop rbx"       "\n"
    //     "pop rcx"       "\n"
    //     "pop rdx"       "\n"
    //     "pop rsi"       "\n"
    //     "pop rdi"       "\n"
    //     "pop rbp"       "\n"
    //     "pop r8"        "\n"
    //     "pop r9"        "\n"
    //     "pop r10"       "\n"
    //     "pop r11"       "\n"
    //     "pop r12"       "\n"
    //     "pop r13"       "\n"
    //     "pop r14"       "\n"
    //     "pop r15"       "\n"
    // );

    // asm volatile("add rsp, 8 * 4"); // 4 fields left in the struct
    // asm("mov %0, rsp" : "=m" (tmp_rsp));

    // asm volatile ("push qword %[ss]" :: [ss]"m"(process_context.ss) : "memory");
    // asm volatile ("push qword %[rsp]" :: [rsp]"m"(tmp_rsp) : "memory");
    // asm volatile ("push qword %[rflags]" :: [rflags]"m"(process_context.flags) : "memory");
    // asm volatile ("push qword %[cs]" :: [cs]"m"(process_context.cs) : "memory");
    // asm volatile ("push qword %[rip]" :: [rip]"m"(process_context.rip) : "memory");

    // asm volatile(
    //         "push rax"      "\n"    // save rax
    
    //         // slave
    //         "mov al, 0x20"  "\n"    // PIC_CMD_EOI
    //         "out 0xA0, al"  "\n"    // PIC_CMD_SLAVE
    //         // master
    //         "mov al, 0x20"  "\n"    // PIC_CMD_EOI
    //         "out 0x20, al"  "\n"    // PIC_CMD_MASTER
            
    //         "pop rax"       "\n"    // restore rax
    // );

    // asm volatile ("iretq");
}