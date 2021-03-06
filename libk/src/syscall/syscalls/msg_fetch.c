#include "../syscalls.h"
#include <schedule/scheduler.h>
#include <memory/virtual_memory_manager.h>
#include <string.h>
#include <schedule/atomic.h>

uint64_t syscall_msg_fetch(const syscall_parameter_t* params)
{
    msg_t msg;
    scheduler_fetch_message(get_current_thread()->parent->pid, &msg);

    msg_t* msg_addr = (msg_t*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    memcpy((void*)msg_addr, (void*)&msg, sizeof(msg_t) - sizeof(uint64_t));
    return msg.sender;
}