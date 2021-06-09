#include <schedule/scheduler.h>
#include <string.h>
#include <memory/page_frame_allocator.h>
#include <device/pit.h>
#include <memory/heap.h>
#include <x86/gdt.h>

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

static void idle_task()
{
    while(true)
        asm("pause");
}

static void* create_stack(uint64_t rip, process_privilege_t privilege)
{
    void* stack = pfa_alloc_page();
    process_context_t* context = (process_context_t*)((uint8_t*)stack + pfa_page_size() - sizeof(process_context_t));
    memset(context, 0, sizeof(process_context_t));
    context->rip = rip;
    context->flags = 0x202;

    if(privilege == PRIVILEGE_USER)
    {
        context->ds = SELECTOR_USER_DATA | 3;
        context->es = SELECTOR_USER_DATA | 3;
        context->fs = SELECTOR_USER_DATA | 3;
        context->gs = SELECTOR_USER_DATA | 3;
        context->ss = SELECTOR_USER_DATA | 3;
        context->cs = SELECTOR_USER_CODE | 3;
    }
    else
    {
        context->ds = SELECTOR_KERNEL_DATA;
        context->es = SELECTOR_KERNEL_DATA;
        context->fs = SELECTOR_KERNEL_DATA;
        context->gs = SELECTOR_KERNEL_DATA;
        context->ss = SELECTOR_KERNEL_DATA;
        context->cs = SELECTOR_KERNEL_CODE;
    }

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

static thread_t create_thread(process_t* parent, entry_point_t entry_point)
{
    parent->thread_count++;

    void* heap_base = pfa_alloc_page();
    void* stack_base = create_stack((uint64_t)entry_point, parent->privilege);
    void* kstack_base = pfa_alloc_page();

    thread_t thread;
    thread.parent = parent;
    thread.heap = heap_create(0xb000000000, pfa_page_size());
    thread.stack_base = 0xa000000000;
    thread.kernel_rsp = 0x9000000000 + pfa_page_size();
    thread.rsp = (uint64_t)thread.stack_base + pfa_page_size() - sizeof(process_context_t);

    paging_map(parent->pagign, heap_base, 0xb000000000, thread.heap.size,
        parent->privilege == PRIVILEGE_KERNEL ? PAGE_PRIVILEGE_KERNEL : PAGE_PRIVILEGE_USER
    );
    paging_map(parent->pagign, stack_base, 0xa000000000, pfa_page_size(),
        parent->privilege == PRIVILEGE_KERNEL ? PAGE_PRIVILEGE_KERNEL : PAGE_PRIVILEGE_USER
    );
    paging_map(parent->pagign, kstack_base, 0x9000000000, pfa_page_size(), PRIVILEGE_KERNEL);

    return thread;
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

// used in schedule_isr.asm
thread_t* get_next_thread()
{
    for(size_t i = current_process + 1; i < MAX_POCESSES; i++)
        if(processes[i].pid != -1)
        {
            current_process = i;
            heap_activate(&(processes[i].threads[0].heap));
            paging_enable(processes[i].pagign);
            tss_set_kstack(processes[i].threads[0].kernel_rsp);
            return &(processes[i].threads[0]);
        }
    current_process = 0;
    heap_activate(&(processes[0].threads[0].heap));
    paging_enable(processes[0].pagign);
    tss_set_kstack(processes[0].threads[0].kernel_rsp);
    return &(processes[0].threads[0]);
}

void scheduler_initialize()
{
    // map in kernel paging process address
    extern paging_data_t kernel_paging;
    paging_map(kernel_paging, 0x9000000000, 0x9000000000, 0x3000000000, PAGE_PRIVILEGE_KERNEL);

    for(size_t i = 0; i < MAX_POCESSES; i++)
    {
        memset(&(processes[i]), 0, sizeof(process_t));
        processes[i].pid = -1;   
    }

    current_process = -1;
    create_process(idle_task, PRIVILEGE_KERNEL);
}

void schedule()
{
    pit_register_callback(schedule_isr);
}

size_t create_process(entry_point_t entry_point, process_privilege_t privilege)
{
    size_t slot = find_process_slot();
    if(slot == -1)
        return -1;

    process_t* process = &(processes[slot]);

    process->pid = slot;
    process->privilege = privilege;
    process->thread_count = 0;
    process->current_thread = 0;
    process->pagign = paging_create();
    extern void* kernel_end;    
    process->threads[0] = create_thread(process, entry_point);

    return slot;
}

bool attach_thread(size_t pid, entry_point_t entry_point)
{
    process_t* process = find_process(pid);
    if(!process)
        return false;

    if(process->thread_count > MAX_THREADS)
        return false;

    process->threads[process->thread_count] = create_thread(processes, entry_point);

    return true;
}

void process_terminate()
{
    process_t* process = &(processes[current_process]);
    for(size_t i = 0; i < process->thread_count; i++)
    {
        thread_t* thread = &(process->threads[i]);
        pfa_free_page((void*)thread->heap.base_address); // free heap memory
        pfa_free_page(thread->stack_base);  // free stack memory
        pfa_free_page(thread->kernel_rsp - pfa_page_size()); // free kernel stack memory
    }
    processes[current_process].pid = -1;

    while(1)
        asm("pause");
}