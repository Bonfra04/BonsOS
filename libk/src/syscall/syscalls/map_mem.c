#include "../syscalls.h"
#include <syscall/syscall.h>
#include <memory/virtual_memory_manager.h>
#include <memory/page_frame_allocator.h>
#include <smp/scheduler.h>

uint64_t syscall_map_mem(const syscall_parameter_t* params)
{
    void* ph_mem = (void*)params->arg0;
    size_t size = (size_t)params->arg1;
    size_t num_page = size / pfa_page_size() + (size % pfa_page_size() != 0);
    void* res;
    if(size == 0)
        res = vmm_translate_vaddr(scheduler_current_thread()->parent->pagign, ph_mem);
    else if(ph_mem == 0)
        res = vmm_alloc_pages(scheduler_current_thread()->parent->pagign, PAGE_PRIVILEGE_USER, num_page);
    else
        res = vmm_assign_pages(scheduler_current_thread()->parent->pagign, PAGE_PRIVILEGE_USER, num_page, ph_mem);

    return (uint64_t)res;
}