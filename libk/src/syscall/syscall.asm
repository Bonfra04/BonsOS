bits 64

section .text
    global syscall_handle
syscall_handle:
    o64 sysret