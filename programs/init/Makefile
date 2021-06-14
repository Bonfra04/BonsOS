DIR_ROOT := ../..
CC := $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-gcc
LD := $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-ld

all:
	@nasm -f elf64 ./src/entry.asm -o ./obj/entry.o
	@$(CC) -std=gnu11 -c -I $(DIR_ROOT)/libc/include -Qn -g \
		-m64 -mno-red-zone -mno-mmx -mfpmath=sse -masm=intel \
		-ffreestanding -fno-asynchronous-unwind-tables -mcmodel=large \
		./src/main.c -o ./obj/main.o
	@$(LD) -nostdlib -T $(DIR_ROOT)/img-content/bin/binary.ld -o init.bin \
		./obj/entry.o ./obj/main.o $(DIR_ROOT)/bin-int/libc/libc.a