#include "../syscalls.h"
#include <smp/scheduler.h>
#include <memory/virtual_memory_manager.h>
#include <string.h>

uint64_t syscall_msg_send(const syscall_parameter_t* params)
{
    msg_t* msg_addr = (msg_t*)vmm_translate_vaddr(scheduler_current_thread()->parent->paging, (void*)params->arg1);
    msg_t msg;
    memcpy((void*)&msg, (void*)msg_addr, sizeof(msg_t) - sizeof(uint64_t));
    msg.sender = scheduler_current_thread()->parent->pid;
    uint64_t send_to = params->arg0;
    scheduler_enqueue_message(send_to, &msg);
    return 0;
}