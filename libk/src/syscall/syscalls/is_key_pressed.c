#include "../syscalls.h"
#include <device/mouse.h>
#include <device/keyboard.h>
#include <memory/virtual_memory_manager.h>

uint64_t syscall_is_key_pressed(const syscall_parameter_t* params)
{
    uint32_t key = (uint32_t)params->r8;
    if(key <= 0xFF)
        return kb_is_key_down((uint8_t)key);
    else
        return mouse_is_down(key - 0xFF);
}