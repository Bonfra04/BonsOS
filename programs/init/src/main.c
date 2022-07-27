#include <stdint.h>

extern int sys(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

int main()
{
    int fd = sys(0, (uint64_t)"a:/mimmo.txt", 1, 0);
    sys(3, fd, (uint64_t)"Hello, world!", 13);
    sys(1, fd, 0, 0);
    return 0;
}
