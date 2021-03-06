#include <schedule/scheduler.h>
#include <string.h>
#include <memory/page_frame_allocator.h>
#include <device/pit.h>
#include <memory/heap.h>
#include <x86/gdt.h>
#include <memory/virtual_memory_manager.h>
#include <schedule/atomic.h>

#define USER_PROCESS_BASE_ADDRESS 0x8000000000

#define USER_STACK_SIZE 256

#define MAX_POCESSES 32

extern void schedule_isr(const interrupt_context_t* interrupt_context);

typedef struct process_context
{
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;

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

    uint64_t ss;
    uint64_t cs;

    uint64_t flags;
    
    uint64_t rip;
} __attribute__ ((packed)) process_context_t;

static process_t processes[MAX_POCESSES];
static size_t current_process;
static process_t* kernel_process;

static void __attribute__((aligned(4096))) idle_task()
{
    while(true)
        scheduler_force_skip();
}

static uint64_t stack_push8(uint64_t rsp, uint8_t value)
{
    rsp -= 1;
    *(uint8_t*)rsp = value;
    return rsp;
}

static uint64_t stack_push64(uint64_t rsp, uint64_t value)
{
    rsp -= 8;
    *(uint64_t*)rsp = value;
    return rsp;
}

static void* create_stack(paging_data_t paging_data, uint64_t rip, int argc, char* argv[], uint64_t* rsp)
{
    void* stack = vmm_alloc_pages(paging_data, PAGE_PRIVILEGE_USER, USER_STACK_SIZE);
    void* ph_stack = vmm_translate_vaddr(paging_data, stack);

    uint64_t sp = (uint64_t)ph_stack + pfa_page_size() * USER_STACK_SIZE;

    // push arguments
    for(int i = 0; i < argc; i++)
    {
        size_t len = strlen(argv[i]);

        // pad to qwrod align
        while((sp - (len + 1)) % 8 != 0)
            sp = stack_push8(sp, 0);

        sp = stack_push8(sp, 0); // null term
        for(int j = len - 1; j >= 0; j--)
            sp = stack_push8(sp, argv[i][j]);

        argv[i] = (uint64_t)stack + (sp - (uint64_t)ph_stack);
    }

    // push pointer to arguments
    for(int i = argc - 1; i >= 0; i--)
        sp = stack_push64(sp, (uint64_t)argv[i]);
    uint64_t argv_ptr = sp;

    // push context
    process_context_t* context = (process_context_t*)(sp -= sizeof(process_context_t));
    memset(context, 0, sizeof(process_context_t));
    
    context->rip = rip;
    context->flags = 0x202;
    context->rdi = argc;
    context->rsi = (uint64_t)stack + (argv_ptr - (uint64_t)ph_stack);

    context->ds = SELECTOR_USER_DATA | 3;
    context->es = SELECTOR_USER_DATA | 3;
    context->fs = SELECTOR_USER_DATA | 3;
    context->gs = SELECTOR_USER_DATA | 3;
    context->ss = SELECTOR_USER_DATA | 3;
    context->cs = SELECTOR_USER_CODE | 3;

    *rsp = (uint64_t)stack + (sp - (uint64_t)ph_stack);

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

static thread_t* run_thread(size_t proc, size_t thread_id)
{
    current_process = proc;
    processes[proc].current_thread = thread_id;
    thread_t* thread = &(processes[proc].threads[thread_id]);
    tss_set_kstack(thread->kernel_rsp);

    void* tmp = vmm_translate_vaddr(processes[proc].pagign, thread->rsp);
    
    extern paging_data_t kernel_paging; 
    if(thread->syscalling)
        paging_enable(kernel_paging);
    else
        paging_enable(processes[proc].pagign);

    return thread;
}

// used in schedule_isr.asm
thread_t* get_next_thread()
{
    extern paging_data_t kernel_paging;
    paging_enable(kernel_paging);
    if(current_process == -1)
        return run_thread(0, 0); // process 0 thread 0

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
    scheduler_force_skip();
}

size_t create_process(entry_point_t entry_point, int argc, char* argv[], size_t size)
{
    size_t slot = find_process_slot();
    if(slot == -1)
        return -1;

    process_t* process = &(processes[slot]);
    process->pid = slot;
    process->thread_count = 0;
    process->current_thread = 0;
    process->pagign = paging_create();

    void* pages = (void*)USER_PROCESS_BASE_ADDRESS;
    for(size_t i = 0; i < size; i++)
        paging_attach_4kb_page(process->pagign, entry_point + pfa_page_size() * i, pages + pfa_page_size() * i, PAGE_PRIVILEGE_USER);

    process->msg_queue = queue_create(msg_t);

    //void* pages = vmm_assign_pages(PAGE_PRIVILEGE_USER, size, entry_point);
    attach_thread(process->pid, pages, argc, argv);

    return slot;
}

const thread_t* get_current_thread()
{
    if(current_process == -1)
        return 0;
    
    process_t* process = &processes[current_process];
    return &(process->threads[process->current_thread]);
}

bool attach_thread(size_t pid, entry_point_t entry_point, int argc, char* argv[])
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

    process->thread_count++;

    thread->parent = process;
    thread->stack_base = create_stack(process->pagign, (uint64_t)entry_point, argc, argv, &thread->rsp);
    thread->kernel_rsp = vmm_alloc_pages(process->pagign, PAGE_PRIVILEGE_KERNEL, 4) + pfa_page_size() * 4;

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
    process_t* process = &(processes[current_process]);
    thread_t* thread = &(process->threads[process->current_thread]);

    vmm_free_pages(process->pagign, thread->stack_base, USER_STACK_SIZE);
    vmm_free_pages(process->pagign, thread->kernel_rsp - pfa_page_size() * 4, 4);

    atomic_start();
    memset(thread, 0, sizeof(thread_t));
    process->thread_count--;
    
    if(process->thread_count == 0)
        process_terminate();

    atomic_end();

    scheduler_force_skip();
}

void process_terminate()
{
    process_t* process = &(processes[current_process]);

    queue_destroy(process->msg_queue);

    atomic_start();
    for(size_t i = 0; i < MAX_THREADS; i++)
        if(process->threads[i].parent)
        {
            thread_t* thread = &(process->threads[i]);
            vmm_free_pages(process->pagign, thread->stack_base, USER_STACK_SIZE);
            vmm_free_pages(process->pagign, thread->kernel_rsp - pfa_page_size() * 4, 4);
            memset(thread, 0, sizeof(thread_t));
            process->thread_count--;
        }
    vmm_destroy(process->pagign);

    memset(process, 0, sizeof(process_t));
    process->pid = -1;
    current_process = -1;
    atomic_end();

    scheduler_force_skip();
}

inline void scheduler_force_skip()
{
    asm("int 0x20");
}

static mutex_t msg_mutex;

void scheduler_enqueue_message(uint64_t pid, msg_t* msg)
{
    mutex_acquire(&msg_mutex);
    process_t* process = find_process(pid);
    __queue_push(process->msg_queue, msg);
    mutext_release(&msg_mutex);
}

void scheduler_fetch_message(uint64_t pid, msg_t* msg)
{
    mutex_acquire(&msg_mutex);
    process_t* process = find_process(pid);
    msg_t* new_msg = queue_front(process->msg_queue);
    if(new_msg == 0)
        memset(msg, 0, sizeof(msg_t));
    else
        memcpy(msg, new_msg, sizeof(msg_t));
    queue_pop(process->msg_queue);
    mutext_release(&msg_mutex);
}

inline void scheduler_toggle_syscall_state()
{
    ((thread_t*)get_current_thread())->syscalling = !get_current_thread()->syscalling;
}

inline paging_data_t get_current_process_paging()
{
    return get_current_thread()->parent->pagign;
}