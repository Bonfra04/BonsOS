#include <stdint.h>
#include <stddef.h>
#include <string.h>

void sys_fwrite(size_t file, void* buffer, size_t len)
{
    asm volatile (
        "mov rax, 4"        "\n" // write_file(
        "mov r8, %[file]"   "\n" //     file,
        "mov r9, %[buffer]" "\n" //     buffer,
        "mov r10, %[len]"   "\n" //     len
        "syscall"           "\n" // );
        : :
        [file]"r"(file),
        [buffer]"r"(buffer),
        [len]"r"(buffer)
    );
}

void main()
{
    const char* str = "Hello, World!";
    sys_fwrite(1, str, strlen(str));
}