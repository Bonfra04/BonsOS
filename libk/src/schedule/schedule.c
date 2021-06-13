#include <schedule/scheduler.h>
#include <string.h>
#include <memory/page_frame_allocator.h>
#include <device/pit.h>
#include <memory/heap.h>
#include <x86/gdt.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/atomic.h>

#define MAX_POCESSES 32

extern void schedule_isr(const interrupt_context_t* interrupt_context);

typedef struct process_context
{
    uint64_t rip;

    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;

    uint64_t ss;
    uint64_t cs;

    uint64_t flags;
} __attribute__ ((packed)) process_context_t;

static process_t processes[MAX_POCESSES];
static volatile size_t current_process;
static process_t* kernel_process;

static void __attribute__((aligned(4096))) idle_task()
{
    while(true)
        asm("pause");
}

static void* create_stack(uint64_t rip)
{
    void* stack = vmm_alloc_page(PAGE_PRIVILEGE_USER);
    void* ph_stack = vmm_translate_vaddr(stack);
    process_context_t* context = (process_context_t*)((uint8_t*)ph_stack + pfa_page_size() - sizeof(process_context_t));
    memset(context, 0, sizeof(process_context_t));
    context->rip = rip;
    context->flags = 0x202;

    context->ds = SELECTOR_USER_DATA | 3;
    context->es = SELECTOR_USER_DATA | 3;
    context->fs = SELECTOR_USER_DATA | 3;
    context->gs = SELECTOR_USER_DATA | 3;
    context->ss = SELECTOR_USER_DATA | 3;
    context->cs = SELECTOR_USER_CODE | 3;

    return stack;
}

static size_t find_process_slot()
{
    for(size_t i = 0; i < MAX_POCESSES; i++)
        if(processes[i].pid == -1)
            return i;
    return -1;
}

static process_t* find_process(size_t pid)
{
    for(int i = 0; i < MAX_POCESSES; i++)
        if(processes[i].pid == pid)
            return &(processes[i]);
    return 0;
}

// used in schedule_isr.asm
void set_thread_stack(size_t rsp)
{
    if(current_process == -1)
        return;
    process_t* process = &(processes[current_process]);
    process->threads[process->current_thread].rsp = rsp;
}

// used in schedule_isr.asm
void set_thread_paging(size_t cr3)
{
    if(current_process == -1)
        return;
    process_t* process = &(processes[current_process]);
    process->pagign = (paging_data_t)cr3;
}

static thread_t* run_thread(size_t proc, size_t thread)
{
    current_process = proc;
    processes[proc].current_thread = thread;
    paging_enable(processes[proc].pagign);
    tss_set_kstack(processes[proc].threads[thread].kernel_rsp);
    return &(processes[proc].threads[thread]);
}

// used in schedule_isr.asm
thread_t* get_next_thread()
{
    if(current_process == -1)
        return run_thread(0, 0);

    for(size_t i = processes[current_process].current_thread + 1; i < MAX_THREADS; i++)
        if(processes[current_process].threads[i].parent)
            return run_thread(current_process, i); 

    for(size_t i = current_process + 1; i < MAX_POCESSES; i++)
        if(processes[i].pid != -1)
            for(size_t j = 0; j < MAX_THREADS; j++)
                if(processes[current_process].threads[j].parent)
                    return run_thread(i, j);

    return run_thread(0, 0);
}

void scheduler_initialize()
{
    for(size_t i = 0; i < MAX_POCESSES; i++)
    {
        memset(&(processes[i]), 0, sizeof(process_t));
        processes[i].pid = -1;   
    }

    current_process = -1;
    kernel_process = &(processes[0]);

    extern paging_data_t kernel_paging;

    kernel_process->pid = 0;
    kernel_process->thread_count = 0;
    kernel_process->current_thread = 0;
    kernel_process->pagign = kernel_paging;

    create_kernel_task(idle_task);
}

void schedule()
{
    pit_register_callback(schedule_isr);
}

size_t create_process(entry_point_t entry_point, size_t size)
{
    size_t slot = find_process_slot();
    if(slot == -1)
        return -1;

    process_t* process = &(processes[slot]);
    process->pid = slot;
    process->thread_count = 0;
    process->current_thread = 0;
    process->pagign = paging_create();

    vmm_set_paging(process->pagign);

    void* pages = vmm_assign_pages(PAGE_PRIVILEGE_USER, size, entry_point);
    attach_thread(process->pid, pages);

    return slot;
}

bool attach_thread(size_t pid, entry_point_t entry_point)
{
    process_t* process = find_process(pid);
    if(!process)
        return false;

    if(process->thread_count >= MAX_THREADS)
        return false;

    thread_t* thread;
    for(size_t i = 0; i < MAX_THREADS; i++)
        if(process->threads[i].parent == 0)
        {
            thread = &(process->threads[i]);
            break;
        }

    vmm_set_paging(process->pagign);

    thread->parent = process;
    thread->stack_base = create_stack((uint64_t)entry_point);
    thread->rsp = (uint64_t)thread->stack_base + pfa_page_size() - sizeof(process_context_t);
    thread->kernel_rsp = vmm_alloc_page(PAGE_PRIVILEGE_KERNEL) + pfa_page_size();

    return true;
}

bool create_kernel_task(entry_point_t entry_point)
{
    if(kernel_process->thread_count >= MAX_THREADS)
        return false;

    kernel_process->thread_count++;

    thread_t* thread;
    for(size_t i = 0; i < MAX_THREADS; i++)
        if(kernel_process->threads[i].parent == 0)
        {
            thread = &(kernel_process->threads[i]);
            break;
        }

    void* stack = pfa_alloc_page();
    process_context_t* context = (process_context_t*)((uint8_t*)stack + pfa_page_size() - sizeof(process_context_t));
    memset(context, 0, sizeof(process_context_t));
    context->rip = (uint64_t)entry_point;
    context->flags = 0x202;

    context->ds = SELECTOR_KERNEL_DATA;
    context->es = SELECTOR_KERNEL_DATA;
    context->fs = SELECTOR_KERNEL_DATA;
    context->gs = SELECTOR_KERNEL_DATA;
    context->ss = SELECTOR_KERNEL_DATA;
    context->cs = SELECTOR_KERNEL_CODE;

    thread->stack_base = stack;
    thread->rsp = (uint64_t)thread->stack_base + pfa_page_size() - sizeof(process_context_t);
    thread->parent = kernel_process;

    return true;
}

void thread_terminate()
{
    atomic_start();

    process_t* process = &(processes[current_process]);
    thread_t* thread = &(process->threads[process->current_thread]);

    vmm_set_paging(process->pagign);

    vmm_free_page(thread->stack_base);
    vmm_free_page(thread->kernel_rsp - pfa_page_size());
    memset(thread, 0, sizeof(thread_t));
    process->thread_count--;
    if(process->thread_count == 0)
        process_terminate();

    atomic_end();
}

void process_terminate()
{
    atomic_start();

    process_t* process = &(processes[current_process]);

    vmm_set_paging(process->pagign);

    for(size_t i = 0; i < MAX_THREADS; i++)
        if(process->threads[i].parent)
        {
            thread_t* thread = &(process->threads[i]);
            vmm_free_page(thread->stack_base);
            vmm_free_page(thread->kernel_rsp - pfa_page_size());
            memset(thread, 0, sizeof(thread_t));
            process->thread_count--;
        }
    memset(process, 0, sizeof(process_t));
    process->pid = -1;
    current_process = -1;

    vmm_destroy();

    atomic_end();

    while(1)
        asm("pause");
}