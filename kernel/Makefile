DIR_ROOT		:= ..
include $(DIR_ROOT)/scripts/config.mk

LIB_NAME		:= kernel
LD_FILE			:= kernel.ld
LIB_DEPS		:= $(call libfile, libk) $(call libfile, klibc) $(call libfile, libbons)

RUNNABLE		:= $(DIR_BIN)/kernel/kernel.sys

include $(DIR_ROOT)/scripts/lib.mk
