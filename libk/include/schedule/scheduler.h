#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <x86/cpu.h>
#include <memory/heap.h>
#include <memory/paging.h>
#include <queue.h>

#define MAX_THREADS 32

typedef void(*entry_point_t)(void);

typedef struct process process_t;

typedef struct msg
{
    uint8_t id;
    uint8_t data[31];
    uint64_t sender;
} __attribute__ ((packed)) msg_t;

typedef struct thread
{
    uint64_t rsp;
    void* stack_base;
    void* kernel_rsp;
    heap_data_t heap;
    process_t* parent;
} __attribute__ ((packed)) thread_t;

typedef struct process
{
    size_t pid;
    paging_data_t pagign;
    size_t thread_count;
    size_t current_thread;
    thread_t threads[MAX_THREADS];
    queue_t msg_queue;
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
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 * @param[in] size amount of pages from the entry point
 * @return the process id (-1 on error)
 */
size_t create_process(entry_point_t entry_point, int argc, char* argv[], size_t size);

/**
 * @brief return a pointer to the current executing thread
 * @return pointer to the thread struct
 */
const thread_t* get_current_thread();

/**
 * @brief attaches thread to a process
 * @param[in] pid id of the process to attach to
 * @param[in] entry_point the entry point of the thread
 * @return success
 */
bool attach_thread(size_t pid, entry_point_t entry_point, int argc, char* argv[]);

/**
 * @brief creates a kernel task and adds it to the queue
 * @param[in] entry_point the entry point of the task
 * @return success
 */
bool create_kernel_task(entry_point_t entry_point);

/**
 * @brief stops the execution of the current thread (calls process_terminate if no more threads are presents)
 */
void thread_terminate();

/**
 * @brief stops the execution of the current process
 */
void process_terminate();

/**
 * @brief skip the time period of this process
 */
void scheduler_force_skip();

/**
 * @brief enqueue a message to a process
 * @param[in] pid id of the process
 * @param[in] msg message to send
 */
void scheduler_enqueue_message(uint64_t pid, msg_t* msg);

/**
 * @brief fetch a message from the process queue
 * @param[in] pid id of the process
 * @param[out] msg address to store the message in
 */
void scheduler_fetch_message(uint64_t pid, msg_t* msg);
