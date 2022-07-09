bits 64

section .text
    global thunks
    extern isr_dispatcher

align 8
thunks:

%assign interrupt 0
%rep 256

align 16
push strict qword interrupt
jmp qword isr_dispatcher

%assign interrupt interrupt+1
%endrep
