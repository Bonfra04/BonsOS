DIR_ROOT		:= ..
include $(DIR_ROOT)/scripts/config.mk

ifdef kernel
LIB_NAME		:= libc
else
PREPROC := -D KERNEL_BUILD
LIB_NAME		:= klibc
endif

include $(DIR_ROOT)/scripts/lib.mk