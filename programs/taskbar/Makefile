DIR_ROOT	:= ../..
include $(DIR_ROOT)/scripts/config.mk

LIB_NAME		:= taskbar
LD_FILE			:= $(DIR_ROOT)/img-content/bin/elf.ld
LIB_DEPS		:= $(call libfile, libusr) $(call libfile, libc) $(call libfile, rund) 

RUNNABLE		:= $(DIR_ROOT)/img-content/bin/taskbar.elf

include $(DIR_ROOT)/scripts/lib.mk
