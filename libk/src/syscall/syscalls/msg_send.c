#include "../syscalls.h"
#include <schedule/scheduler.h>
#include <memory/virtual_memory_manager.h>
#include <string.h>

uint64_t syscall_msg_send(const syscall_parameter_t* params)
{
    msg_t* msg_addr = (msg_t*)vmm_translate_vaddr((void*)params->r9);
    msg_t msg;
    memcpy((void*)&msg, (void*)msg_addr, sizeof(msg_t) - sizeof(uint64_t));
    msg.sender = get_current_thread()->parent->pid;
    uint64_t send_to = params->r8;
    scheduler_enqueue_message(send_to, &msg);
    return 0;
}