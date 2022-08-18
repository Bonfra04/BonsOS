DIR_ROOT	:= ../..
include $(DIR_ROOT)/scripts/config.mk

include $(DIR_ROOT)/img-content/bin/flags.mk

LIB_NAME		:= shell
LD_FILE			:= $(DIR_ROOT)/img-content/bin/elf.ld
LIB_DEPS		:= $(call libfile, libc)  $(call libfile, libbons)

CCFLAGS := $(CCFLAGS) -I $(DIR_PROGRAMS)/$(LIB_NAME)/include

RUNNABLE		:= $(DIR_ROOT)/img-content/bin/shell.elf

include $(DIR_ROOT)/scripts/lib.mk
