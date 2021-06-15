DIR_ROOT := ../

include $(DIR_ROOT)/scripts/config.mk

.PHONY: all init clean


all: clean init

init:
	@make $(MAKE_FLAGS) --directory=./init
	@mv ./init/init.bin $(DIR_ROOT)/img-content/bin/init.bin

clean:
	@make clean $(MAKE_FLAGS) --directory=./init
	@rm -rf $(DIR_ROOT)/img-content/bin/init.bin