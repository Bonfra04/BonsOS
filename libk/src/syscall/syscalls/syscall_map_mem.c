#include <memory/vmm.h>
#include <memory/pfa.h>
#include <smp/scheduler.h>

void* syscall_map_mem(void* ph_addr, size_t size)
{
    size_t num_pages = size / PFA_PAGE_SIZE + (size % PFA_PAGE_SIZE != 0);
    if(ph_addr == NULL)
        return vmm_alloc(current_thread->proc->paging, PAGE_PRIVILEGE_USER, num_pages);
    else
        return vmm_assign_pages(current_thread->proc->paging, PAGE_PRIVILEGE_USER, num_pages, ph_addr);
}
