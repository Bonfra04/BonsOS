#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <x86/cpu.h>
#include <memory/heap.h>

#define MAX_THREADS 32

typedef enum process_privilege
{
    PRIVILEGE_KERNEL = 0,
    PRIVILEGE_USER = 1,
} process_privilege_t;

typedef void(*entry_point_t)(void);

typedef struct process process_t;

typedef struct thread
{
    uint64_t rsp;
    void* stack_base;
    heap_data_t heap;
    process_t* parent;
} __attribute__ ((packed)) thread_t;

typedef struct process
{
    size_t pid;
    process_privilege_t privilege;
    size_t thread_count;
    size_t current_thread;
    thread_t threads[MAX_THREADS];
} __attribute__ ((packed))process_t;

/**
 * @brief initialize the scheduler
 */
void scheduler_initialize();

/**
 * @brief start scheduling
 */
void schedule();

/**
 * @brief creates a process and adds it to the queue
 * @param[in] entry_point the entry point of the main thread
 * @param[in] privilege the process privilege
 * @return the process id (-1 on error)
 */
size_t create_process(entry_point_t entry_point, process_privilege_t privilege);

/**
 * @brief attaches thread to a process
 * @param[in] pid id of the process to attach to
 * @param[in] entry_point the entry point of the thread
 * @return success
 */
bool attach_thread(size_t pid, entry_point_t entry_point);

/**
 * @brief stops the execution of the current process
 */
void process_terminate();
