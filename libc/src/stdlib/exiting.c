#include <stdlib.h>
#include <stdint.h>

#include <containers/linked_list.h>

static linked_list_t atexit_functions;
static linked_list_t at_quick_exit_functions;

static void __attribute__((constructor)) __exiting_init()
{
    atexit_functions = linked_list_new();
    at_quick_exit_functions = linked_list_new();
}

int atexit(void (*func)())
{
    linked_list_append(atexit_functions, void*, func);
    return 0;
}

int at_quick_exit(void (*func)())
{
    linked_list_append(at_quick_exit_functions, void*, func);
    return 0;
}

void exit(int status)
{
    size_t len = linked_list_size(atexit_functions);
    for (size_t i = len - 1; i < len; i--)
    {
        void* func = linked_list_val(atexit_functions, void*, i);
        ((void(*)())func)();
    }

    _Exit(status);
}

void quick_exit(int status)
{
    size_t len = linked_list_size(at_quick_exit_functions);
    for (size_t i = len - 1; i < len; i--)
    {
        void* func = linked_list_val(at_quick_exit_functions, void*, i);
        ((void(*)())func)();
    }

    _Exit(status);
}

#ifdef KERNEL_BUILD

#include <panic.h>
#include <cpu.h>
#include <log.h>

void _Exit(int status)
{
    if(status != 0)
        kernel_panic("Exiting kernel with status %d", status);

    kernel_trace("Exiting kernel successfully");
    for(;;)
    {
        cli();
        hlt();
    }
}

#else

#include <syscalls.h>

extern void call_dtors();

void _Exit(int status)
{
    (void)status; // TODO: use status

    call_dtors();

    sys_process_exit();
    for(;;);
}

#endif
