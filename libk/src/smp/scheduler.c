#include <smp/scheduler.h>
#include <memory/vmm.h>
#include <memory/pfa.h>
#include <interrupts/lapic.h>
#include <memory/gdt.h>
#include <fsys/fsys.h>
#include <cpu.h>
#include <panic.h>

#include <linker.h>
#include <alignment.h>
#include <containers/darray.h>

#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#define LAPIC_ISR 0x20

#define THREAD_STACK_SIZE 256
#define USER_PROCESS_BASE_ADDRESS 0x8000000000

static const size_t resources_size[] = {
    [RES_FILE] = sizeof(file_t),
    [RES_UNUSED] = 0
};

static process_t* kernel_process;
thread_t* current_thread;

extern void scheduler_tick(const interrupt_context_t* context);
extern void scheduler_replace_switch(thread_t* thread);

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

static void* create_stack(const process_t* proc, void* vt_stack_base, void* vt_entry, bool is_kernel, char* args[])
{
    uint64_t ph_stack_base = (uint64_t)paging_get_ph(proc->paging, vt_stack_base);
    uint64_t sp = ph_stack_base + THREAD_STACK_SIZE * PFA_PAGE_SIZE;

    uint64_t argv_ptr = 0;
    size_t argc = 0;
    if(args)
    {
        for(; args[argc]; argc++);
        char* arg_ptrs[argc];

        // push arguments
        for(size_t i = 0; i < argc; i++)
        {
            size_t len = strlen(args[i]);

            // pad to qwrod align
            while((sp - (len + 1)) % 8 != 0)
                sp = stack_push8(sp, 0);

            sp = stack_push8(sp, 0); // null term
            for(int j = len - 1; j >= 0; j--)
                sp = stack_push8(sp, args[i][j]);

            arg_ptrs[i] = vt_stack_base + (sp - ph_stack_base);
        }

        // pad to qwrod align
        while(sp % 8 != 0)
            sp = stack_push8(sp, 0);

        // push pointer to arguments
        for(int i = argc - 1; i >= 0; i--)
            sp = stack_push64(sp, (uint64_t)arg_ptrs[i]);

        argv_ptr = sp;
    }

    interrupt_context_t* context = (interrupt_context_t*)(sp -= sizeof(interrupt_context_t));
    memset(context, 0, sizeof(interrupt_context_t));

    context->retaddr = (uint64_t)vt_entry;
    context->rflags = 0x202;
    context->registers.rdi = argc;
    context->registers.rsi = (uint64_t)vt_stack_base + (argv_ptr - ph_stack_base);

    context->ds = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->es = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->fs = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->gs = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->ss = is_kernel ? SELECTOR_KERNEL_DATA : (SELECTOR_USER_DATA | 3);
    context->cs = is_kernel ? SELECTOR_KERNEL_CODE : (SELECTOR_USER_CODE | 3);

    return ptr(context->rsp = (uint64_t)vt_stack_base + (sp - ph_stack_base));
}

void scheduler_init()
{
    kernel_process = malloc(sizeof(process_t));
    kernel_process->paging = kernel_paging;
    kernel_process->executable = NULL;
    kernel_process->threads = darray(thread_t*, 0);
    kernel_process->resources = darray(resource_t, 0);
    kernel_process->workdir = "";

    current_thread = malloc(sizeof(thread_t));
    current_thread->proc = kernel_process;
    current_thread->next_thread = current_thread;
    current_thread->prev_thread = current_thread;
    darray_append(kernel_process->threads, current_thread);
}

void scheduler_start()
{
    asm volatile ("mov %0, rsp" : "=r"(current_thread->rsp) : : "memory");

    isr_set(LAPIC_ISR, scheduler_tick);
    scheduler_yield();
}

static void thread_insert(thread_t* thread)
{
    scheduler_atomic({
        thread->next_thread = current_thread->next_thread;
        thread->prev_thread = current_thread;
        current_thread->next_thread->prev_thread = thread;
        current_thread->next_thread = thread;
    });
}

static void thread_destroy(thread_t* thread)
{
    process_t* proc = thread->proc;

    if(thread->stack_base)
        vmm_free(proc->paging, thread->stack_base, THREAD_STACK_SIZE);
    if(thread->kstack_base)
        pfa_free(thread->kstack_base, THREAD_STACK_SIZE);
    free(thread);

    darray_remove(proc->threads, darray_find(proc->threads, thread));
}

static void process_cleanup(process_t* process)
{
    if(process == kernel_process)
        return;

    vmm_destroy(process->paging);
    executable_unload(process->executable);

    for(size_t i = 0; i < darray_length(process->threads); i++)
        thread_destroy(process->threads[i]);

    darray_destroy(process->threads);
    free((char*)process->workdir);

    for(size_t i = 0; i < darray_length(process->resources); i++)
    {
        resource_t* resource = &process->resources[i];
        void* data = resource->resource;
        switch (resource->type)
        {
        case RES_FILE:
            fsys_close_file(data);
            break;
        }
        free(data);
    }
    darray_destroy(process->resources);

    free(process);
}

static void process_init(process_t* proc, void* address_low, void* address_high, const char* workdir)
{
    proc->paging = paging_create();
    proc->threads = darray(thread_t*, 0);
    proc->workdir = strdup(workdir);
    proc->resources = darray(resource_t, 0);

    address_low = ptr(ALIGN_4K_DOWN(address_low));
    address_high = ptr(ALIGN_4K_UP(address_high));

    size_t size = address_high - address_low;
    paging_map(proc->paging, address_low, ptr(USER_PROCESS_BASE_ADDRESS), size, PAGE_PRIVILEGE_USER);

    extern symbol_t __kernel_start_addr;
    extern symbol_t __kernel_end_addr;
    size_t kernel_start_aligned = ALIGN_4K_DOWN(__kernel_start_addr);
    size_t kernel_end_aligned = ALIGN_4K_UP(__kernel_end_addr);
    size_t kernel_size = kernel_end_aligned - kernel_start_aligned;
    paging_map(proc->paging, __kernel_start_addr, ptr(kernel_start_aligned), kernel_size, PAGE_PRIVILEGE_KERNEL);
}

static void inherit_resourcess(process_t* parent, process_t* child)
{
    resource_t* parent_resources = parent->resources;
    for(size_t i = 0; i < darray_length(parent_resources); i++)
    {
        resource_t* parent_res = &parent_resources[i];
        
        resource_t res;
        res.type = parent_res->type;
        res.resource = malloc(resources_size[parent_res->type]);
        memcpy(res.resource, parent_res->resource, resources_size[parent_res->type]);

        darray_append(child->resources, res);
    }
}

process_t* scheduler_run_executable(const executable_t* executable, const char* workdir, const char* args[], const char* env[])
{
    // TODO: use env

    void* address_low = executable->base_address;
    void* address_high = executable->num_pages * PFA_PAGE_SIZE + executable->base_address;
    void* vt_entry = ptr(USER_PROCESS_BASE_ADDRESS + (uint64_t)executable->entry_point - (uint64_t)executable->base_address);

    process_t* proc = malloc(sizeof(process_t));
    proc->executable = executable;

    process_init(proc, address_low, address_high, workdir);
    inherit_resourcess(current_thread->proc, proc);

    scheduler_attach_thread(proc, vt_entry, args);

    return proc;
}

void scheduler_attach_thread(process_t* proc, void* entry_point, const char* args[])
{
    size_t argc = 0;
    if(args)
        for(; args[argc]; argc++);
    char* argv[argc + 2];
    argv[0] = proc->executable->fullpath;
    memcpy(argv + 1, args, sizeof(char*) * argc);
    argv[argc + 1] = NULL;

    void* stack_base = vmm_alloc(proc->paging, PAGE_PRIVILEGE_USER, THREAD_STACK_SIZE);
    void* rsp = create_stack(proc, stack_base, entry_point, false, argv);

    thread_t* new = malloc(sizeof(thread_t));
    new->rsp = (uint64_t)rsp;
    new->proc = proc;
    new->stack_base = stack_base;

    new->kstack_base = pfa_alloc(THREAD_STACK_SIZE);
    new->krsp = (uint64_t)new->kstack_base + THREAD_STACK_SIZE * PFA_PAGE_SIZE;
    paging_map(proc->paging, new->kstack_base, new->kstack_base, THREAD_STACK_SIZE * PFA_PAGE_SIZE, PAGE_PRIVILEGE_KERNEL);

    darray_append(proc->threads, new);
    thread_insert(new);
}

void scheduler_create_kernel_task(void* entry_point)
{
    void* stack_base = pfa_alloc(THREAD_STACK_SIZE);
    void* rsp = create_stack(kernel_process, stack_base, entry_point, true, NULL);

    thread_t* new = malloc(sizeof(thread_t));
    new->krsp = new->rsp = (uint64_t)rsp;
    new->proc = kernel_process;
    new->kstack_base = new->stack_base = stack_base;

    darray_append(kernel_process->threads, new);
    thread_insert(new);
}

void scheduler_terminate_process()
{
    scheduler_atomic({
        if(current_thread == current_thread->next_thread)
            kernel_panic("No more threads to schedule");

        process_t* proc = current_thread->proc;

        while(current_thread->proc == proc)
        {
            current_thread->prev_thread->next_thread = current_thread->next_thread;
            current_thread->next_thread->prev_thread = current_thread->prev_thread;
            current_thread = current_thread->next_thread;
        }

        process_cleanup(proc);

        scheduler_replace_switch(current_thread);
        kernel_panic("Process termination failed");
    });
}

void scheduler_terminate_thread()
{
    scheduler_atomic({
        if(current_thread == current_thread->next_thread)
            kernel_panic("No more threads to schedule");

        process_t* proc = current_thread->proc;
        thread_t* old;

        current_thread->prev_thread->next_thread = current_thread->next_thread;
        current_thread->next_thread->prev_thread = current_thread->prev_thread;

        old = current_thread;
        current_thread = current_thread->next_thread;

        thread_destroy(old);
        if(darray_length(proc->threads) == 0)
            process_cleanup(proc);

        scheduler_replace_switch(current_thread);
        kernel_panic("Thread termination failed");
    });
}

int scheduler_alloc_resource(void* resource, resource_type_t type)
{
    resource_t* resources = current_thread->proc->resources;
    for(size_t i = 0; i < darray_length(resources); i++)
        if(resources[i].type == RES_UNUSED)
        {
            resources[i].type = type;
            resources[i].resource = resource;
            return i;
        }

    resource_t new;
    new.type = type;
    new.resource = resource;
    darray_append(current_thread->proc->resources, new);
    return darray_length(current_thread->proc->resources) - 1;
}

void* scheduler_get_resource(int id, resource_type_t type)
{
    resource_t* resources = current_thread->proc->resources;
    if(id >= darray_length(resources))
        return NULL;

    if(resources[id].type != type)
        return NULL;

    return resources[id].resource;
}

bool scheduler_free_resource(int id, resource_type_t type)
{
    resource_t* resources = current_thread->proc->resources;
    if(id >= darray_length(resources))
        return false;

    if(resources[id].type != type)
        return false;

    resources[id].type = RES_UNUSED;
    resources[id].resource = NULL;

    return true;
}

void scheduler_yield()
{
    asm("int 0x20");
}

thread_t* get_next_thread()
{
    return current_thread->next_thread;
}
