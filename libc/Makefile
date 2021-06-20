DIR_ROOT		:= ..

ifdef kernel
LIB_NAME		:= libc
else
PREPROC := -D KERNEL_BUILD
LIB_NAME		:= klibc
endif

include $(DIR_ROOT)/scripts/lib.mk