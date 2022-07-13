DIR_ROOT		:= ../..
include $(DIR_ROOT)/scripts/config.mk

LIB_NAME		:= loader
LD_FILE			:= stage2.ld

RUNNABLE		:= $(DIR_BIN)/boot/loader.bin
CRT0			:= loader.asm

ASFLAGS	:= -F dwarf -g -f elf32
CCFLAGS	:= -std=gnu11 -Qn -g \
			-m32 -mno-red-zone -mno-mmx -mfpmath=sse -masm=intel \
			-ffreestanding -fno-asynchronous-unwind-tables \
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough -Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context
LDFLAGS	:= -z max-page-size=0x100 -m elf_i386 -nostdlib

POST_BUILD_RULE := separate_symbols

include $(DIR_ROOT)/scripts/lib.mk

separate_symbols:
	@$(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-objcopy --only-keep-debug $(DIR_BIN)/boot/loader.bin $(DIR_BIN)/boot/loader.dbg
	@$(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-objcopy --strip-debug --strip-unneeded $(DIR_BIN)/boot/loader.bin
	@$(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-objcopy -O binary $(DIR_BIN)/boot/loader.bin $(DIR_BIN)/boot/loader.bin