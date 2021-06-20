#include <schedule/atomic.h>

inline void atomic_start()
{
    asm volatile ("cli");
}

inline void atomic_end()
{
    asm volatile ("sti");
}
