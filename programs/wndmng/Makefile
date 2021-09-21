DIR_ROOT	:= ../..
include $(DIR_ROOT)/scripts/config.mk

include $(DIR_ROOT)/img-content/bin/flags.mk

LIB_NAME		:= wndmng
LD_FILE			:= $(DIR_ROOT)/img-content/bin/elf.ld
LIB_DEPS		:= $(call libfile, libusr) $(call libfile, libc) 

RUNNABLE		:= $(DIR_ROOT)/img-content/bin/wndmng.elf

include $(DIR_ROOT)/scripts/lib.mk
