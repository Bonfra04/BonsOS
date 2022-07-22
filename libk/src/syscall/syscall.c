#include <syscall/syscall.h>
#include <memory/gdt.h>
#include <cpu.h>

extern void syscall_handle();

void syscall_init()
{
}

void syscall_enable()
{
    uint64_t efer = rdmsr(MSR_IA32_EFER);
    efer |= (1 << 0);
    wrmsr(MSR_IA32_EFER, efer);

    uint64_t star = rdmsr(MSR_IA32_STAR);
    star &= 0x00000000ffffffff;

    star |= (uint64_t)SELECTOR_KERNEL_CODE << 32;               // syscall CS <- IA32_STAR[47:32]
    star |= (uint64_t)((SELECTOR_USER_CODE - 16) | 3) << 48;    //  sysret CS <- IA32_STAR[63:48]+16
    wrmsr(MSR_IA32_STAR, star);

    wrmsr(MSR_IA32_LSTAR, (uint64_t)syscall_handle);

    wrmsr(MSR_IA32_FMASK, UINT32_MAX);
}
