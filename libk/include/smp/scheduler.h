#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory/paging.h>
#include <queue.h>

#define MAX_THREADS 32

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
    process_t* parent;
    bool syscalling;
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

typedef void(*entry_point_t)(void);

/**
 * @brief intiialize the scheduler, called once by the bsp
 */
void scheduler_init();

/**
 * @brief prepare the scheduling data for the current core (auto called by init)
 */
void scheduler_prepare();

/**
 * @brief start a scheduler for the calling core
 */
void scheduler_start();

bool create_kernel_task(entry_point_t entry_point);

void scheduler_force_skip();

const thread_t* scheduler_current_thread();

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

void scheduler_toggle_syscall_state();

/**
 * @brief creates a process and adds it to the queue
 * @param[in] entry_point the entry point of the main thread
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 * @param[in] size amount of pages from the entry point
 * @param[in] core the core to run the process on
 * @return the process id (-1 on error)
 */
uint64_t scheduler_create_process(entry_point_t entry_point, int argc, char* argv[], size_t size, uint8_t core);

/**
 * @brief attaches thread to a process
 * @param[in] pid id of the process to attach to
 * @param[in] entry_point the entry point of the thread
 * @return success
 */
bool scheduler_attach_thread(size_t pid, entry_point_t entry_point, int argc, char* argv[]);
