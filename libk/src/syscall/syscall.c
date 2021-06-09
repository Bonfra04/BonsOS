#include <syscall/syscall.h>
#include <x86/cpu.h>
#include <x86/gdt.h>
#include <memory/paging.h>

// Model-specific registers used to set up system calls.
#define MSR_IA32_STAR   0xc0000081
#define MSR_IA32_LSTAR  0xc0000082
#define MSR_IA32_FMASK  0xc0000084

extern void syscall_handle();

#define MAX_SYSCALL 128

syscall_t syscalls[MAX_SYSCALL];

#include <stdio.h>

static int i = 0;

static void empty_syscall(syscall_parameter_t* params)
{
    printf("%d.", i++);
}

void syscall_register(size_t id, syscall_t systemcall)
{
    if(id >= MAX_SYSCALL)
        return;
    syscalls[id] = systemcall;
}

void syscall_init()
{
    // Request the CPU's extended features.
    registers4_t regs;
    cpuid(0x80000001, &regs);

    // Bit 11 of rdx tells us if the SYSCALL/SYSRET instructions are available
    if (!(regs.rdx & (1 << 11)))
        invalid_opcode();

    // Update the IA32_STAR MSR with the segment selectors that will be used by SYSCALL and SYSRET.
    uint64_t star = rdmsr(MSR_IA32_STAR);
    star &= 0x00000000ffffffff;
    star |= (uint64_t)SELECTOR_KERNEL_CODE << 32;
    star |= (uint64_t)((SELECTOR_USER_CODE - 16) | 3) << 48;
    wrmsr(MSR_IA32_STAR, star);

    // Write the address of the system call handler used by SYSCALL.
    wrmsr(MSR_IA32_LSTAR, (uint64_t)syscall_handle);

    // Write the CPU flag mask used during SYSCALL.
    wrmsr(MSR_IA32_FMASK, 0);

    for(size_t i = 0; i < MAX_SYSCALL; i++)
        syscalls[i] = empty_syscall;
}