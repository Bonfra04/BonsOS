#pragma once

#include <stdint.h>

typedef struct interrupt_context
{
    uint64_t ds;
    uint64_t es;
    uint64_t fs;
    uint64_t gs;

    struct
    {
        uint64_t rax;
        uint64_t rbx;
        uint64_t rcx;
        uint64_t rdx;
        uint64_t rsi;
        uint64_t rdi;
        uint64_t rbp;
        uint64_t r8;
        uint64_t r9;
        uint64_t r10;
        uint64_t r11;
        uint64_t r12;
        uint64_t r13;
        uint64_t r14;
        uint64_t r15;
    } __attribute__ ((packed)) registers;

    uint64_t error;
    uint64_t interrupt;
    uint64_t retaddr;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__ ((packed)) interrupt_context_t;

typedef void (*isr_handler_t)(const interrupt_context_t *context);

/**
 * @brief registers an isr handler for a specific interrupt
 * @param[in] interrupt the interrupt number
 * @param[in] handler the handler to register
 */
void isr_set(uint8_t interrupt, isr_handler_t handler);
