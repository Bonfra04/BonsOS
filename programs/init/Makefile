DIR_ROOT := ../..
CC := $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-gcc
LD := $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-ld

.PHONY: all build mkdir clean

all: clean mkdir build

build:
	@nasm -f elf64 ./src/entry.asm -o ./obj/entry.o
	@$(CC) -std=gnu11 -c -I $(DIR_ROOT)/libc/include -Qn \
		-m64 -mno-red-zone -mno-mmx -mfpmath=sse -masm=intel \
		-ffreestanding -fno-asynchronous-unwind-tables -mcmodel=large \
		./src/main.c -o ./obj/main.o
	@$(LD) -nostdlib -T $(DIR_ROOT)/img-content/bin/elf.ld -o init.elf -s \
		./obj/entry.o ./obj/main.o $(DIR_ROOT)/bin-int/libc/libc.a -z max-page-size=0x1000

mkdir:
	@mkdir -p ./obj

clean:
	@rm -rf ./init.bin
	@rm -rf ./obj
