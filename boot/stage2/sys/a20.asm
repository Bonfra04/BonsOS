global a20_kb

a20_kb:
    ; Attempt enabling with the keyboard controller.
    call .wait1

    ; Disable keyboard
    mov al, 0xad
    out 0x64, al
    call .wait1

    ; Read from input
    mov al, 0xd0
    out 0x64, al
    call .wait2

    ; Get keyboard data
    in al, 0x60
    push eax
    call .wait1

    ; Write to output
    mov al, 0xd1
    out 0x64, al
    call .wait1

    ; Send data
    pop eax
    or al, 2
    out 0x60, al
    call .wait1

    ; Enable keyboard
    mov al, 0xae
    out 0x64, al
    call .wait1

    jmp .done

    .wait1:
        in al, 0x64
        test al, 2
        jnz .wait1
        ret

    .wait2:
        in al, 0x64
        test al, 1
        jz .wait2
        ret

.done:
    ret