#----------------------------------------------------------------------------
#   DIR_ROOT	The root directory of the project.
#----------------------------------------------------------------------------

DIR_OBJ		:= $(DIR_ROOT)/bin-int
DIR_BIN		:= $(DIR_ROOT)/bin
DIR_BOOT	:= $(DIR_ROOT)/boot
DIR_KERNEL	:= $(DIR_ROOT)/kernel
DIR_LIBK	:= $(DIR_ROOT)/libk
DIR_LIBC	:= $(DIR_ROOT)/libc
DIR_SCRIPTS	:= $(DIR_ROOT)/scripts

DIRS_INCLUDE := -I $(DIR_ROOT)/libc/include -I $(DIR_ROOT)/libk/include

CC		:=  $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-gcc
CCFLAGS	:= -std=gnu11 $(DIRS_INCLUDE) -Qn -g \
			-m64 -mno-red-zone -mno-mmx -mfpmath=sse -masm=intel \
			-ffreestanding -fno-asynchronous-unwind-tables \
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough -Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context
#-fms-extensions

AS		:= nasm
ASFLAGS	:= -f elf64

LD		:=  $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-ld
LDFLAGS	:= -nostdlib -z max-page-size=0x1000

MAKE_FLAGS := --no-print-directory

BLUE	:= \033[1;34m
YELLOW	:= \033[1;33m
NORMAL	:= \033[0m

SUCCESS	:= $(YELLOW)SUCCESS$(NORMAL)

# Calculate path of lib file from lib name
libfile = $(join $(1:%=$(DIR_OBJ)/%), $(1:%=/%.a))