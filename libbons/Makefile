DIR_ROOT		:= ..
include $(DIR_ROOT)/scripts/config.mk

ifdef kernel
LIB_NAME		:= libbons
else
PREPROC := -D KERNEL_BUILD
LIB_NAME		:= klibbons
endif

include $(DIR_ROOT)/scripts/lib.mk