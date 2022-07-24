#include <linker.h>

typedef void (*ctor_t)();

void call_ctors()
{
    extern symbol_t __start_ctors, __end_ctors;
    for(ctor_t* ctor = (ctor_t*)&__start_ctors; ctor < (ctor_t*)&__end_ctors; ctor++)
        if(*ctor)
            (*ctor)();
}
