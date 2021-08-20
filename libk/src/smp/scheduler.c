#include <smp/scheduler.h>
#include <interrupt/apic.h>
#include <string.h>
#include <x86/gdt.h>
#include <smp/atomic.h>
#include <stdlib.h>
#include <panic.h>
#include <smp/smp.h>
#include <memory/virtual_memory_manager.h>
#include <memory/page_frame_allocator.h>
#include <smp/lts.h>

#define _PID(core_id, local_pid) (((core_id) << 56) | (local_pid))
#define PID(local_pid) _PID(lapic_get_id(), local_pid)

#define THREAD_STACK_SIZE 256
#define MAX_PROCESSES 256 * 2

static process_t processes[256][MAX_PROCESSES];
static uint64_t current_processes[256];
#define current_process (uint64_t)(locked_read(&current_processes[lapic_get_id()]))
#define current_process_set(process) locked_write(&current_processes[lapic_get_id()], process)
#define get_process(index) (processes[lapic_get_id()][index])
static mutex_t scheduler_mutex;

#define USER_PROCESS_BASE_ADDRESS 0x8000000000

static void __attribute__((aligned(4096))) idle_task()
{
    while(true)
        scheduler_force_skip();
}

static uint64_t find_slot()
{
    for(uint64_t i = 0; i < MAX_PROCESSES; i++)
        if(locked_read(&get_process(i).thread_count) == 0)
            return i;

    kenrel_panic("out of process slot");

    return -1;
}

static process_t* find_process(size_t pid)
{
    uint64_t core_id = pid >> 56;
    uint64_t local_pid = pid & 0xFFFFFFFFFFFFFF;
    if(core_id > 0xFF || local_pid >= MAX_PROCESSES)
        return 0;
    return &processes[core_id][local_pid];
}

static thread_t* run_thread(size_t proc, size_t thread_id)
{
    current_process_set(proc);
    process_t* process = &get_process(proc);

    locked_write(&process->current_thread, thread_id);
    thread_t* thread = &process->threads[thread_id];
    tss_set_kstack(thread->kernel_rsp);

    return thread;
}

static const thread_t* run_next_thread()
{
    if(current_process == -1)
        return run_thread(0, 0); // process 0 thread 0

    process_t* procs = processes[lapic_get_id()];

    for(uint64_t i = locked_read(&procs[current_process].current_thread) + 1; i < MAX_THREADS; i++)
        if(locked_read(&procs[current_process].threads[i].parent))
            return run_thread(current_process, i); 

    for(size_t i = current_process + 1; i < MAX_PROCESSES; i++)
        for(size_t j = 0; j < MAX_THREADS; j++)
            if(locked_read(&procs[i].threads[j].parent) != 0)
                return run_thread(i, j);

    return run_thread(0, 0);
}

static uint64_t __attribute__ ((aligned(16))) stack[1024];

static __attribute__((naked)) void scheduler_isr(const interrupt_context_t* context)
{
    extern paging_data_t kernel_paging;

    if(current_process != -1) 
    {
        register process_t* process = &get_process(current_process);
        asm volatile ("mov %0, rsp" : "=r"(process->threads[process->current_thread].rsp) : : "memory");
        register uint64_t rsp = process->threads[process->current_thread].rsp;
        asm volatile("mov rsp, %[addr]" : : [addr]"r"(&stack[1023]) : "memory");
        asm volatile("mov cr3, %[addr]" : : [addr]"r"(kernel_paging) : "memory");
        rsp = (void*)paging_get_ph(process->pagign, (void*)rsp);
        asm volatile("mov rsp, %[addr]" : : [addr]"r"(rsp) : "memory");
        process->threads[process->current_thread].rsp += 9 * 8; // remove stack frame
    }
    register thread_t* thread = run_next_thread();
    LAPIC_ISR_DONE();
    asm volatile("mov rsp, %[addr]" : : [addr]"r"(locked_read(&thread->rsp)) : "memory");
    if(locked_read(&thread->syscalling))
        asm volatile("mov cr3, %[addr]" : : [addr]"r"(kernel_paging) : "memory");
    else
        asm volatile("mov cr3, %[addr]" : : [addr]"r"(locked_read(&thread->parent->pagign)) : "memory");
    asm volatile("jmp restore_context");
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

// int c = 0;
static void* create_stack(paging_data_t paging_data, uint64_t rip, int argc, char* argv[], uint64_t* rsp)
{
    void* stack = vmm_alloc_pages(paging_data, PAGE_PRIVILEGE_USER, THREAD_STACK_SIZE);
    void* ph_stack = vmm_translate_vaddr(paging_data, stack);

    uint64_t sp = (uint64_t)ph_stack + pfa_page_size() * THREAD_STACK_SIZE;

    char* trans_argv[argc];

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

        trans_argv[i] = (uint64_t)stack + (sp - (uint64_t)ph_stack);
    }

    // push pointer to arguments
    for(int i = argc - 1; i >= 0; i--)
        sp = stack_push64(sp, (uint64_t)trans_argv[i]);
    uint64_t argv_ptr = sp;

    // push context
    interrupt_context_t* context = (interrupt_context_t*)(sp -= sizeof(interrupt_context_t));
    memset(context, 0, sizeof(interrupt_context_t));

    context->retaddr = rip;
    context->rflags = 0x202;
    context->regs.rdi = argc;
    context->regs.rsi = (uint64_t)stack + (argv_ptr - (uint64_t)ph_stack);

    // c++;
    // if (c == 2)
    //     context->rflags = 0b10;

    context->ds = SELECTOR_USER_DATA | 3;
    context->es = SELECTOR_USER_DATA | 3;
    context->fs = SELECTOR_USER_DATA | 3;
    context->gs = SELECTOR_USER_DATA | 3;
    context->ss = SELECTOR_USER_DATA | 3;
    context->cs = SELECTOR_USER_CODE | 3;
    
    context->rsp = *rsp = (uint64_t)stack + (sp - (uint64_t)ph_stack);

    return stack;
}

void scheduler_init()
{
    memset(processes, 0, sizeof(processes));

    scheduler_prepare();
}

void scheduler_prepare()
{
    mutex_acquire(&scheduler_mutex);

    extern paging_data_t kernel_paging;    
    uint8_t core_id = lapic_get_id();

    processes[core_id][0].pid = PID(0); 
    processes[core_id][0].pagign = kernel_paging;
    processes[core_id][0].thread_count = 0;
    processes[core_id][0].current_thread = 0;
    memset(processes[core_id][0].threads, 0, sizeof(processes[core_id][0].threads));
    processes[core_id][0].msg_queue = queue_create(msg_t);

    current_process_set(-1);

    mutext_release(&scheduler_mutex);

    create_kernel_task(idle_task);
}

void scheduler_start()
{
    lapic_timer_callback(scheduler_isr);

    scheduler_force_skip();
}

bool create_kernel_task(entry_point_t entry_point)
{
    mutex_acquire(&scheduler_mutex);

    process_t* kernel_process = &get_process(0);

    if(locked_read(&kernel_process->thread_count) >= MAX_THREADS)
    {
        mutext_release(&scheduler_mutex);
        return false;
    }

    kernel_process->thread_count++;

    thread_t* thread;
    for(size_t i = 0; i < MAX_THREADS; i++)
        if(kernel_process->threads[i].parent == 0)
        {
            thread = &(kernel_process->threads[i]);
            break;
        }

    void* stack = pfa_alloc_pages(THREAD_STACK_SIZE);
    interrupt_context_t* context = (interrupt_context_t*)((uint8_t*)stack + pfa_page_size() * THREAD_STACK_SIZE - sizeof(interrupt_context_t));
    memset(context, 0, sizeof(interrupt_context_t));
    context->retaddr = (uint64_t)entry_point;
    context->rflags = 0x202;

    context->ds = SELECTOR_KERNEL_DATA;
    context->es = SELECTOR_KERNEL_DATA;
    context->fs = SELECTOR_KERNEL_DATA;
    context->gs = SELECTOR_KERNEL_DATA;
    context->ss = SELECTOR_KERNEL_DATA;
    context->cs = SELECTOR_KERNEL_CODE;

    context->rsp = (uint64_t)stack + pfa_page_size() * THREAD_STACK_SIZE;

    thread->stack_base = stack;
    thread->rsp = context->rsp - sizeof(interrupt_context_t);
    thread->kernel_rsp = 0;
    thread->parent = kernel_process;
    thread->syscalling = false;

    mutext_release(&scheduler_mutex);

    return true;
}

inline void scheduler_force_skip()
{
    asm("int 0x20");
}

inline const thread_t* scheduler_current_thread()
{
    if(current_process == -1)
        return 0;
    process_t* process = &get_process(current_process);
    return &process->threads[locked_read(&process->current_thread)];
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

void scheduler_toggle_syscall_state()
{
    ((thread_t*)scheduler_current_thread())->syscalling = !scheduler_current_thread()->syscalling;
}

uint64_t scheduler_create_process(entry_point_t entry_point, int argc, char* argv[], size_t size, uint8_t core)
{
    mutex_acquire(&scheduler_mutex);

    uint64_t lpid = find_slot();
    uint64_t pid = PID(lpid);

    process_t* process = &get_process(lpid);
    process->pid = pid;
    process->thread_count = 0;
    process->current_thread = -1;
    process->pagign = paging_create();

    void* pages = (void*)USER_PROCESS_BASE_ADDRESS;
    for(size_t i = 0; i < size; i++)
        paging_attach_4kb_page(process->pagign, entry_point + pfa_page_size() * i, pages + pfa_page_size() * i, PAGE_PRIVILEGE_USER);

    process->msg_queue = queue_create(msg_t);

    mutext_release(&scheduler_mutex);

    scheduler_attach_thread(process->pid, pages, argc, argv);

    return pid;
}

bool scheduler_attach_thread(size_t pid, entry_point_t entry_point, int argc, char* argv[])
{
    mutex_acquire(&scheduler_mutex);

    process_t* process = find_process(pid);
    if(process == 0)
    {
        mutext_release(&scheduler_mutex);
        return false;
    }

    thread_t* thread;
    for(size_t i = 0; i < MAX_THREADS; i++)
        if(locked_read(&process->threads[i].parent) == 0)
        {
            thread = &(process->threads[i]);
            break;
        }

    void* kstack_base = pfa_alloc_pages(THREAD_STACK_SIZE);

    thread->parent = process;
    thread->syscalling = false;
    thread->kernel_rsp = kstack_base + pfa_page_size() * THREAD_STACK_SIZE;
    thread->stack_base = create_stack(process->pagign, (uint64_t)entry_point, argc, argv, &thread->rsp);

    // vmm_assign_pages(process->pagign, PAGE_PRIVILEGE_KERNEL, THREAD_STACK_SIZE, kstack_base);

    paging_map(process->pagign, kstack_base, kstack_base, THREAD_STACK_SIZE * pfa_page_size(), PAGE_PRIVILEGE_KERNEL);

    process->thread_count++;

    mutext_release(&scheduler_mutex);
    return true;
}

paging_data_t scheduler_get_current_paging()
{
    return locked_read(&get_process(current_process).pagign);
}