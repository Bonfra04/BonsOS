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
