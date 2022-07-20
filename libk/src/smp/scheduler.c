#include <smp/scheduler.h>
#include <memory/vmm.h>
#include <memory/pfa.h>
#include <interrupts/lapic.h>
#include <memory/gdt.h>
#include <cpu.h>
#include <panic.h>

#include <alignment.h>

#include <stdlib.h>
#include <string.h>

#define LAPIC_ISR 0x20

#define THREAD_STACK_SIZE 256

static process_t* kernel_process;
thread_t* current_thread;

extern void scheduler_tick(const interrupt_context_t* context);
extern void scheduler_replace_switch(thread_t* thread);

static void* create_stack(const process_t* proc, void* vt_stack_base, void* vt_entry, bool is_kernel)
{
    uint64_t ph_stack_base = (uint64_t)paging_get_ph(proc->paging, vt_stack_base);
    uint64_t sp = ph_stack_base + THREAD_STACK_SIZE * PFA_PAGE_SIZE;
    
    interrupt_context_t* context = (interrupt_context_t*)(sp -= sizeof(interrupt_context_t));
    memset(context, 0, sizeof(interrupt_context_t));

    context->retaddr = (uint64_t)vt_entry;
    context->rflags = 0x202;

    context->ds = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->es = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->fs = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->gs = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->ss = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->cs = is_kernel ? SELECTOR_KERNEL_CODE : (SELECTOR_USER_CODE | 3);
    
    return context->rsp = (uint64_t)vt_stack_base + (sp - ph_stack_base);
}

static void destroy_process(process_t* process)
{
    vmm_destroy(process->paging);
}

static void destroy_thread(thread_t* thread)
{
    process_t* proc = current_thread->proc;
    
    if(thread->stack_base)
        vmm_free(proc->paging, thread->stack_base, THREAD_STACK_SIZE);
    free(thread);

    proc->n_threads--;
    if(proc->n_threads == 0)
        destroy_process(proc);
}

void scheduler_init()
{
    kernel_process = malloc(sizeof(process_t));
    paging_get_current(kernel_process->paging);
    kernel_process->n_threads = 0;
}

void scheduler_start()
{
    current_thread = malloc(sizeof(thread_t));
    kernel_process->n_threads++;

    asm volatile ("mov %0, rsp" : "=r"(current_thread->rsp) : : "memory");
    current_thread->proc = kernel_process;
    current_thread->next_thread = current_thread;
    current_thread->prev_thread = current_thread;

    isr_set(LAPIC_ISR, scheduler_tick);
    scheduler_yield();
}

void scheduler_create_kernel_task(void* entry_point)
{
    void* stack_base = pfa_alloc(THREAD_STACK_SIZE);
    void* rsp = create_stack(kernel_process, stack_base, entry_point, true);

    thread_t* new = malloc(sizeof(thread_t));
    new->rsp = (uint64_t)rsp;
    new->proc = kernel_process;
    new->stack_base = stack_base;

    scheduler_atomic({
        new->next_thread = current_thread->next_thread;
        new->prev_thread = current_thread;
        current_thread->next_thread->prev_thread = new;
        current_thread->next_thread = new;
        kernel_process->n_threads++;
    });
}

void scheduler_yield()
{
    asm("int 0x20");
}

thread_t* get_next_thread()
{
    return current_thread->next_thread;
}

void scheduler_terminate_thread()
{
    thread_t* old;
    scheduler_atomic({
        current_thread->prev_thread->next_thread = current_thread->next_thread;
        current_thread->next_thread->prev_thread = current_thread->prev_thread;

        old = current_thread;
        current_thread = current_thread->next_thread;
    });

    destroy_thread(old);

    if(current_thread == old)
        kernel_panic("No more threads to schedule");

    scheduler_replace_switch(current_thread);
    kernel_panic("Thread termination failed");
}
