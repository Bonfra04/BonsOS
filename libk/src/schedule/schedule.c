#include <schedule/scheduler.h>
#include <string.h>
#include <memory/page_frame_allocator.h>

#define KERNEL_DATA 0x08
#define KERNEL_CODE 0x10

#define MAX_POCESSES 32

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

static void idle_task()
{
    while(true)
        asm("pause");
}

static void* create_stack(uint64_t rip)
{
    void* stack = pfa_alloc_page() + pfa_get_page_size();
    process_context_t* context = (uint8_t*)stack - sizeof(process_context_t);
    memset(context, 0, sizeof(process_context_t));
    context->rip = rip;

    context->cs = KERNEL_CODE;
	context->ds = KERNEL_DATA;
	context->es = KERNEL_DATA;
	context->fs = KERNEL_DATA;
	context->gs = KERNEL_DATA;

    return (uint8_t*)stack - sizeof(process_context_t);
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
    thread.rsp = (uint64_t)create_stack((uint64_t)entry_point);
    thread.ss = KERNEL_DATA;
    return thread;
}

void scheduler_initialize()
{
    for(size_t i = 0; i < MAX_POCESSES; i++)
    {
        memset(&(processes[i]), 0, sizeof(process_t));
        processes[i].pid = -1;   
    }

    //create_process(idle_task, PRIVILEGE_KERNEL);
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

void execute_process(size_t pid)
{
    process_t* process = &(processes[pid]);
    
    // maybe theese two values should be saved in a register and then edited 'cause they are from the stack
    asm("mov ss, %[value]" : : [value]"X"(process->threads[0].ss));
    asm("mov rsp, %[value]" : : [value]"X"(process->threads[0].rsp));

    asm(        
        "pop rax"   "\n"
        "pop rbx"   "\n"
        "pop rcx"   "\n"
        "pop rdx"   "\n"
        "pop rsi"   "\n"
        "pop rdi"   "\n"
        "pop rbp"   "\n"
        "pop r8"    "\n"
        "pop r9"    "\n"
        "pop r10"   "\n"
        "pop r11"   "\n"
        "pop r12"   "\n"
        "pop r13"   "\n"
        "pop r14"   "\n"
        "pop r15"   "\n"

        "pop rax"   "\n"//cr3

        "pop rax"   "\n"
        "mov ds, rax""\n"
        "pop rax"   "\n"
        "mov es, rax""\n"
        "pop fs"    "\n"
        "pop gs"    "\n"
    );

    // stack has rip and cs
    asm("retfq");

    while(1);
}