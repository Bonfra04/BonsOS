#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <x86/cpu.h>
#include <memory/heap.h>

#define MAX_THREADS 32

typedef enum process_privilege
{
    PRIVILEGE_KERNEL,
    PRIVILEGE_USER,
} process_privilege_t;

typedef void(*entry_point_t)(void);

typedef struct process process_t;

typedef struct thread
{
    uint64_t rsp;
    uint64_t ss;
    heap_data_t heap;
    process_t* parent;
} thread_t;

typedef struct process
{
    size_t pid;
    process_privilege_t privilege;
    size_t thread_count;
    thread_t threads[MAX_THREADS];
} process_t;

/**
 * @brief initialize the scheduler
 */
void scheduler_initialize();

/**
 * @brief creates and run a process
 * @param[in] entry_point the entry point of the main thread
 * @param[in] privilege the process privilege
 * @return the process id (-1 on error)
 */
size_t create_process(entry_point_t entry_point, process_privilege_t privilege);

/**
 * @brief attaches thread to a process and runs it
 * @param[in] pid id of the process to attach to
 * @param[in] entry_point the entry point of the thread
 * @return success
 */
bool attach_thread(size_t pid, entry_point_t entry_point);
