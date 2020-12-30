#include <dump.h>
#include <stdio.h>

static const char digit[] = "0123456789ABCDEF";

int dump_registers(char* buf, size_t bufsize, const registers_t* regs)
{
    return snprintf
    (
        buf, bufsize,
        "RAX: %016lx    RSI: %016lx    R11: %016lx\n"
        "RBX: %016lx    RDI: %016lx    R12: %016lx\n"
        "RCX: %016lx     R8: %016lx    R13: %016lx\n"
        "RDX: %016lx     R9: %016lx    R14: %016lx\n"
        "RBP: %016lx    R10: %016lx    R15: %016lx\n",
        regs->rax, regs->rsi, regs->r11,
        regs->rbx, regs->rdi, regs->r12,
        regs->rcx, regs->r8, regs->r13,
        regs->rdx, regs->r9, regs->r14,
        regs->rbp, regs->r10, regs->r15
    );
}

int dump_cpuflags(char* buf, size_t bufsize, uint64_t rflags)
{
#define B(F)  ((rflags & F) ? 1 : 0)

    return snprintf
    (
        buf, bufsize,
        "CF=%u   PF=%u   AF=%u   ZF=%u   SF=%u   "
        "TF=%u   IF=%u   DF=%u   OF=%u   IOPL=%u\n",
        B(CPU_EFLAGS_CARRY), B(CPU_EFLAGS_PARITY), B(CPU_EFLAGS_ADJUST),
        B(CPU_EFLAGS_ZERO), B(CPU_EFLAGS_SIGN), B(CPU_EFLAGS_TRAP),
        B(CPU_EFLAGS_INTERRUPT), B(CPU_EFLAGS_DIRECTION),
        B(CPU_EFLAGS_OVERFLOW), (rflags >> 12) & 3
    );

#undef B
}

int dump_memory(char* buf, size_t bufsize, const void* mem, size_t memsize, enum dumpstyle style)
{
    char* b = buf;
    char * bt = buf + bufsize;
    const uint8_t* m  = (const uint8_t*)mem;
    const uint8_t* mt = (const uint8_t*)mem + memsize;

    while (b < bt && m < mt)
    {
        if (style == DUMPSTYLE_OFFSET)
        {
            if (b + 11 < bt)
            {
                uint64_t o = (uint64_t)(m - (const uint8_t*)mem);
                for (int i = 7; i >= 0; i--)
                {
                    b[i] = digit[o & 0xf];
                    o  >>= 4;
                }
                b[8] = ':';
                b[9] = b[10] = ' ';
            }
            b += 11;
        }
        else if (style == DUMPSTYLE_ADDR)
        {
            if (b + 20 < bt)
            {
                uint64_t a = (uint64_t)m;
                for (int i = 16; i > 8; i--)
                {
                    b[i] = digit[a & 0xf];
                    a  >>= 4;
                }
                b[8] = '`';
                for (int i = 7; i >= 0; i--)
                {
                    b[i] = digit[a & 0xf];
                    a  >>= 4;
                }
                b[17] = ':';
                b[18] = b[19] = ' ';
            }
            b += 20;
        }

        for (int j = 0; j < 16; j++)
        {
            if (b + 2 < bt)
            {
                if (m + j < mt)
                {
                    uint8_t v = m[j];
                    b[0] = digit[v >> 4];
                    b[1] = digit[v & 0xf];
                }
                else
                    b[0] = b[1] = ' ';
            }
            b += 2;

            if (((j + 1) & 3) == 0)
            {
                if (b + 1 < bt)
                    *b = ' ';
                b++;
            }
        }

        if (b + 1 < bt)
            *b = ' ';
        b++;

        for (int j = 0; j < 16; j++)
        {
            if (b + 1 < bt)
                if (m + j < mt)
                {
                    uint8_t v = m[j];
                    *b = (v < 32 || v > 126) ? '.' : (char)v;
                }
                else
                    *b = ' ';
            b++;

            if (j == 7)
            {
                if (b + 1 < bt)
                    *b = ' ';
                b++;
            }
        }

        if (b + 1 < bt)
            *b = '\n';
        b++;

        m += 16;
    }

    if (b < bt)
        *b = 0;
    else if (bufsize > 0)
        b[bufsize - 1] = 0;
        
    return (int)(b - buf);
}