#pragma once

#include <memory/paging.h>
#include <executable/executable.h>
#include <cpu.h>

#include <stdnoreturn.h>
#include <stdint.h>

typedef struct process process_t;

typedef struct thread thread_t;
typedef struct thread
{
    uint64_t rsp;
    void* stack_base;

    uint64_t krsp;
    void* kstack_base;

    process_t* proc;

    thread_t* next_thread;
    thread_t* prev_thread;
} thread_t;

typedef enum resource_type
{
    RES_FILE, RES_UNUSED
} resource_type_t;

typedef struct resource
{
    resource_type_t type;
    void* resource;
} resource_t;

typedef struct process
{
    paging_data_t paging;
    const executable_t* executable;
    resource_t* resources;
    thread_t** threads;
    const char* workdir;
} process_t;

extern thread_t* current_thread;

/**
 * @brief Initializes the scheduler
 */
void scheduler_init();

/**
 * @brief Starts scheduling the processes
 */
void scheduler_start();

/**
 * @brief Creates a new process with a single thread, mapping the given executable
 * @param[in] executable The executable to load
 * @param[in] workdir Working directory of the process
 * @param[in] args Null terminated string array of arguments
 * @param[in] env Null terminated string array of environment variables
 * @return The created process
 */
process_t* scheduler_run_executable(const executable_t* executable, const char* workdir, char* args[], char* env[]);

/**
 * @brief Attaches a thread to a process
 * @param[in] proc The process to attach the thread to
 * @param[in] entry_point Virtual address of the entry point inside the process
 * @param[in] args Null terminated string array of arguments
 */
void scheduler_attach_thread(process_t* proc, void* entry_point, char* args[]);

/**
 * @brief Creates a task that runs in kernel space
 * @param[in] entry_point Virtual address of the entry point inside the process
 */
void scheduler_create_kernel_task(void* entry_point);

/**
 * @brief Yields the calling thread time slice to the next thread, forcing a context switch
 */
void scheduler_yield();

/**
 * @brief Terminates the calling thread
 */
noreturn void scheduler_terminate_thread();

/**
 * @brief Terminates the calling process
 */
noreturn void scheduler_terminate_process();

/**
 * @brief Executes a block of code ensuring that the scheduler won't tick while it is executing
 * @param[in] code The code to execute
 */
#define scheduler_atomic(code) {                        \
    bool __is_sti = get_flags() & CPU_EFLAGS_INTERRUPT; \
    if(__is_sti)                                        \
        asm volatile ("cli");                           \
    code;                                               \
    if(__is_sti)                                        \
        asm volatile ("sti");                           \
}

/**
 * @brief Allocates a resource for the calling thread
 * @param[in] resource Pointer to the resource to allocate
 * @param[in] type Type of the resource
 * @return The id of the resource
 * @note The resource is not owned by the scheduler
 */
int scheduler_alloc_resource(void* resource, resource_type_t type);

/**
 * @brief returns a resource for the calling thread
 * @param[in] id The id of the resource
 * @param[in] type Expected type of the resource
 * @return The resource
 */
void* scheduler_get_resource(int id, resource_type_t type);

/**
 * @brief Removes a resource for the calling thread
 * @param[in] id The id of the resource
 * @param[in] type Expected type of the resource
 * @return True if the resource was removed, false otherwise
 */
bool scheduler_free_resource(int id, resource_type_t type);
