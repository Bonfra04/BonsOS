#pragma once

#include <memory/paging.h>

#include <stdint.h>

typedef struct process process_t;

typedef struct thread thread_t;
typedef struct thread
{
    uint64_t rsp;
    void* stack_base;

    process_t* proc;

    thread_t* next_thread;
    thread_t* prev_thread;
} thread_t;

typedef struct process
{
    paging_data_t paging;
    uint64_t n_threads;
} process_t;

/**
 * @brief Initializes the scheduler
 */
void scheduler_init();

/**
 * @brief Starts scheduling the processes
 */
void scheduler_start();

/**
 * @brief Creates a new process with a single thread, mapping the given memory area
 * @param[in] address_low Lowest physical address of the process
 * @param[in] address_high Highest physical address of the process
 * @param[in] entry_point Physical address of the entry point
 * @return The created process
 */
process_t* scheduler_create_process(void* address_low, void* address_high, void* entry_point);

/**
 * @brief Attaches a thread to a process
 * @param[in] proc The process to attach the thread to
 * @param[in] entry_point Virtual address of the entry point inside the process
 */
void scheduler_attach_thread(process_t* proc, void* entry_point);

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
void __attribute__((noreturn)) scheduler_terminate_thread();

#define scheduler_atomic(code) { asm volatile ("cli"); code; asm volatile ("sti"); }
