#include <stdint.h>
#include <string.h>

extern int sys(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

int main()
{
    int fd = sys(0, (uint64_t)"tty:/0", 1, 0);

    sys(3, fd, (uint64_t)(void*)"What's your name? ", 18);
    
    char name[5];
    memset(name, '\0', 5);
    sys(2, fd, (uint64_t)(void*)name, 4);
    
    sys(3, fd, (uint64_t)(void*)"Hello ", 6);
    sys(3, fd, (uint64_t)(void*)name, strlen(name));
    sys(3, fd, (uint64_t)(void*)".", 1);

    sys(1, fd, 0, 0);
    return 0;
}
