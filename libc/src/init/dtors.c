#include <linker.h>

typedef void (*dtor_t)();

void call_dtors()
{
    extern symbol_t __start_dtors, __end_dtors;
    for(dtor_t* dtor = (dtor_t*)&__start_dtors; dtor < (dtor_t*)&__end_dtors; dtor++)
        if(*dtor)
            (*dtor)();
}
