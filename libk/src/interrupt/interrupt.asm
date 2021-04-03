bits 64

section .text
    extern isr_handlers

    global isr_dispatcher
    global isr_dispatcher_special

;-----------------------------------------------------------------------------
; ISR.Dispatcher
;
; A general-purpose ISR dispatch routine that all ISR thunks jump to when an
; interrupt arrives. The dispatcher receives from the stack an interrupt
; number, which it uses the to look up a kernel-defined ISR. If a valid ISR is
; found, the dispatcher calls it with a pointer to an interrupt context, which
; contains the contents on all general-purpose CPU registers, the interrupt
; number, and the error code if any.
;-----------------------------------------------------------------------------
isr_dispatcher:
    ; Push a dummy error code.
    push 0

    ; Preserve the first two general-purpose registers.
    push r15
    push r14

    .specialEntry:          ; Entry for ISR.Dispatcher.Special
        ; Preserve the rest of the general-purpose registers.
        push r13
        push r12
        push r11
        push r10
        push r9
        push r8
        push rbp
        push rdi
        push rsi
        push rdx
        push rcx
        push rbx
        push rax

        ; Preserve the MXCSR register.
        sub rsp, 16
        stmxcsr [rsp]

    .lookup:
        ; Look up the kernel-defined ISR in the table.
        mov rax, [rsp + 8 * 18]             ; rax=interrupt number
        mov rax, [isr_handlers + 8 * rax]   ; rax=ISR address

        ; If there is no ISR, then we're done.
        cmp rax, 0
        je .done

    .dispatch:
        ; The System V ABI requires the direction flag to be cleared on
        ; function entry.
        cld

        ; The interrupt context is on the stack, so pass the ISR a pointer to
        ; the stack as the first parameter.
        lea rdi, [rsp + 16]   ; skip the MXCSR register.

        ; Call the ISR.
        call rax

    .done:
        ; Restore the MXCSR register.
        ldmxcsr [rsp]
        add rsp, 16

        ; Restore general-purpose registers.
        pop rax
        pop rbx
        pop rcx
        pop rdx
        pop rsi
        pop rdi
        pop rbp
        pop r8
        pop r9
        pop r10
        pop r11
        pop r12
        pop r13
        pop r14
        pop r15
        add rsp, 16      ; Chop error code and interrupt #

        iretq

;-----------------------------------------------------------------------------
; ISR.Dispatcher.Special
;
; A special dispatcher is used for exceptions 8 and 10 through 14. The CPU
; pushes an error code onto the stack before calling these exceptions'
; interrupt handlers. So, to be compatible with the normal ISR.Dispatcher
; routine, we need to swap the places of the thunk-placed interrupt number and
; the error code on the stack.
;-----------------------------------------------------------------------------
isr_dispatcher_special:
    ; First preserve r14 and r15.
    push r15
    push r14

    ; Use r14 and r15 to swap the interrupt number and error code entries on
    ; the stack.
    mov r14, [rsp + 8 * 2]    ; interrupt
    mov r15, [rsp + 8 * 3]    ; error code
    mov [rsp + 8 * 2], r15
    mov [rsp + 8 * 3], r14

    ; Jump directly to the normal dispatcher, but just beyond the step
    ; that inserts a dummy error code and preserves r14 and r15.
    jmp isr_dispatcher.specialEntry