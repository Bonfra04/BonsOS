bits 64

section .text
    global mutex_acquire
    global mutex_release
    extern scheduler_yield

mutex_acquire:
.try_acquire:
    xor rax, rax
    lock bts [rdi], rax
    jnc .done
    call scheduler_yield ; replace block
    jmp .try_acquire
.done:
    ret

mutex_release:
    xor rax, rax
    lock btr [rdi], rax
    call scheduler_yield ; replace block
    ret
