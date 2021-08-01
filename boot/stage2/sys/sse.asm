bits 32

section .text
    global sse_enable

sse_enable:
    mov eax, cr0
    and eax, ~(1 << 2)   ; turn off CR0.EM bit (x87 FPU is present)
    or eax, (1 << 1)     ; turn on CR0.MP bit (monitor FPU)
    mov cr0, eax

    ; Enable the use of SSE instructions.
    mov eax, cr4
    or eax, (1 << 9) | (1 << 10)    ; CR4.OFXSR, CR4.OSXMMEXCPT
    mov cr4, eax

    ret