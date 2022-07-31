#include <stdint.h>
#include <string.h>
#include <syscalls.h>

int main()
{
    int fd = sys_open_file("tty:/0", OPEN_READ);

    sys_write_file(fd, "What's your name? ", 18);
    
    char name[5];
    memset(name, '\0', 5);
    sys_read_file(fd, name, 4);
    
    sys_write_file(fd, "Hello ", 6);
    sys_write_file(fd, name, strlen(name));
    sys_write_file(fd, ".", 1);

    sys_close_file(fd);
    return 0;
}
