#include "../syscalls.h"
#include <syscall/syscall.h>
#include <memory/virtual_memory_manager.h>
#include <memory/page_frame_allocator.h>

uint64_t syscall_map_mem(const syscall_parameter_t* params)
{
    void* ph_mem = (void*)params->r8;
    size_t size = (size_t)params->r9;
    size_t num_page = size / pfa_page_size() + (size % pfa_page_size() != 0);
    return (uint64_t)vmm_assign_pages(PAGE_PRIVILEGE_USER, num_page, ph_mem);
}