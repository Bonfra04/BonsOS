#include <schedule/scheduler.h>
#include <string.h>
#include <memory/page_frame_allocator.h>
#include <device/pit.h>
#include <memory/heap.h>

#define KERNEL_DATA 0x08
#define KERNEL_CODE 0x10

#define MAX_POCESSES 32

extern void schedule_isr(const interrupt_context_t* interrupt_context);

typedef struct process_context
{
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

    uint64_t cr3;

    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;

    uint64_t rip;
    uint64_t cs;
} __attribute__ ((packed)) process_context_t;

static process_t processes[MAX_POCESSES];
static volatile size_t current_process;

static void idle_task()
{
    while(true)
        asm("pause");
}

static void* create_stack(uint64_t rip)
{
    void* stack = pfa_alloc_page();
    process_context_t* context = (process_context_t*)((uint8_t*)stack + pfa_get_page_size() - sizeof(process_context_t));
    memset(context, 0, sizeof(process_context_t));
    context->rip = rip;

    context->cs = KERNEL_CODE;
	context->ds = KERNEL_DATA;
	context->es = KERNEL_DATA;
	context->fs = KERNEL_DATA;
	context->gs = KERNEL_DATA;

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
    thread_t thread;
    thread.parent = parent;
    thread.heap = heap_create(pfa_alloc_page(), pfa_get_page_size());
    thread.stack_base = create_stack((uint64_t)entry_point);
    thread.rsp = thread.stack_base + pfa_get_page_size() - sizeof(process_context_t);
    thread.ss = KERNEL_DATA;
    return thread;
}

// used in schedule_isr.asm
void set_thread_stack(size_t ss, size_t rsp, size_t thread_id)
{
    processes[current_process].threads[thread_id].ss = ss;
    processes[current_process].threads[thread_id].rsp = rsp;
}

// used in schedule_isr.asm
thread_t* get_next_thread()
{
    for(size_t i = current_process + 1; i < MAX_POCESSES; i++)
        if(processes[i].pid != -1)
        {
            current_process = i;
            heap_activate(&(processes[i].threads[0].heap));
            return &(processes[i].threads[0]);
        }
    current_process = 0;
    heap_activate(&(processes[0].threads[0].heap));
    return &(processes[0].threads[0]);
} 

void scheduler_initialize()
{
    for(size_t i = 0; i < MAX_POCESSES; i++)
    {
        memset(&(processes[i]), 0, sizeof(process_t));
        processes[i].pid = -1;   
    }

    current_process = create_process(idle_task, PRIVILEGE_KERNEL);
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
    process->thread_count = 1;
    process->threads[0] = create_thread(processes, entry_point);

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
    process->thread_count++;

    return true;
}

void process_terminate()
{
    process_t* process = &(processes[current_process]);
    for(size_t i = 0; i < process->thread_count; i++)
    {
        thread_t* thread = &(process->threads[i]);
        pfa_free_page(thread->heap.base_address); // free heap memory
        pfa_free_page(thread->stack_base);  // free stack memory;
    }
    processes[current_process].pid = -1;

    while(1)
        asm("pause");
}