#----------------------------------------------------------------------------
#   DIR_ROOT	The root directory of the project.
#----------------------------------------------------------------------------

DIR_OBJ		:= $(DIR_ROOT)/bin-int
DIR_BIN		:= $(DIR_ROOT)/bin
DIR_BOOT	:= $(DIR_ROOT)/boot
DIR_KERNEL	:= $(DIR_ROOT)/kernel
DIR_LIBK	:= $(DIR_ROOT)/libk
DIR_LIBC	:= $(DIR_ROOT)/libc
DIR_LIBBONS	:= $(DIR_ROOT)/libbons
DIR_SCRIPTS	:= $(DIR_ROOT)/scripts
DIR_PROGRAMS:= $(DIR_ROOT)/programs

DIRS_INCLUDE := -I $(DIR_LIBC)/include -I $(DIR_LIBK)/include -I $(DIR_LIBBONS)/include

CC		:=  $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-gcc
CCFLAGS	:= -std=gnu17 $(DIRS_INCLUDE) -g \
			-D BONSOS \
			-mno-red-zone -mno-mmx -mno-sse -masm=intel \
			-ffreestanding -mcmodel=large \
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough -Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context -Wno-override-init -Wno-switch
#-fms-extensions

AS		:= nasm
ASFLAGS	:= -F dwarf -g -f elf64

LD		:=  $(DIR_ROOT)/tools/cross-compiler/bin/x86_64-elf-ld
LDFLAGS	:= -nostdlib -z max-page-size=0x1000

MAKE_FLAGS := --no-print-directory

BLUE	:= $$(tput setaf 4)$$(tput bold)
YELLOW	:= $$(tput setaf 3)
NORMAL	:= $$(tput sgr0)

SUCCESS	:= $(YELLOW)SUCCESS$(NORMAL)

# Calculate path of lib file from lib name
libfile = $(join $(1:%=$(DIR_OBJ)/%), $(1:%=/%.a))