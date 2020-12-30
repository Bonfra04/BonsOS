DIR_ROOT		:= ..
LIB_NAME		:= kernel
POST_LIB_RULE	:= kernel
LD_FILE			:= kernel.ld

include $(DIR_ROOT)/scripts/lib.mk
include $(DIR_ROOT)/scripts/config.mk

.PHONY: kernel dir

kernel: dir $(DIR_BIN)/kernel/kernel.sys

dir:
	@mkdir $(DIR_BIN)/kernel

$(DIR_BIN)/kernel/kernel.sys: $(LD_FILE) $(LIB_FILE)
	@echo "$(TAG) Linking $(notdir $@)"
	@$(LD) $(LDFLAGS) -T $(LD_FILE) -o $@ $(LIB_FILE) $(call libfile, libk) $(call libfile, libc) 
	@chmod a-x $@
