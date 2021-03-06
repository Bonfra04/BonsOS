#include "../syscalls.h"
#include <device/mouse.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/atomic.h>
#include <schedule/scheduler.h>

uint64_t syscall_get_mouse(const syscall_parameter_t* params)
{
    uint64_t* mouse_x = (void*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r8);
    uint64_t* mouse_y = (void*)vmm_translate_vaddr(get_current_thread()->parent->pagign, (void*)params->r9);
    *mouse_x = mouse_get_x();
    *mouse_y = mouse_get_y();
    return 0;
}