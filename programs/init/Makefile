DIR_ROOT	:= ../..
include $(DIR_ROOT)/scripts/config.mk

LIB_NAME		:= init
LD_FILE			:= $(DIR_ROOT)/img-content/bin/elf.ld
LIB_DEPS		:= $(call libfile, libusr) $(call libfile, libc) 

RUNNABLE		:= $(DIR_ROOT)/img-content/bin/init.elf

include $(DIR_ROOT)/scripts/lib.mk
