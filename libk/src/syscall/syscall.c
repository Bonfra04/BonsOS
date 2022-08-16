#include <syscall/syscall.h>
#include <memory/gdt.h>
#include <cpu.h>

#include "syscalls.h"

#define MAX_SYSCALLS 11

extern void syscall_handle();

void* syscall_handlers[MAX_SYSCALLS];

void syscall_init()
{
    syscall_handlers[0] = syscall_open_file;
    syscall_handlers[1] = syscall_close_file;
    syscall_handlers[2] = syscall_read_file;
    syscall_handlers[3] = syscall_write_file;
    syscall_handlers[4] = syscall_map_mem;
    syscall_handlers[5] = syscall_thread_exit;
    syscall_handlers[6] = syscall_process_exit;
    syscall_handlers[7] = syscall_exec;
    syscall_handlers[8] = syscall_getcwd;
    syscall_handlers[9] = syscall_setcwd;
    syscall_handlers[10] = syscall_delete_file;
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
