#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum executable_format
{
    BINARY,
    ELF,
    PE,
} executable_format_t;

typedef struct msg
{
    uint8_t id;
    uint8_t data[31];
} __attribute__ ((packed)) msg_t;

uint64_t run_executable(const char* path, int argc, char* argv[], executable_format_t format);

void* map_mem(void* ph_mem, size_t size);

void msg_send(uint64_t pid, msg_t* msg);
uint64_t msg_fetch(msg_t* msg);
