DIR_ROOT := ../

include $(DIR_ROOT)/scripts/config.mk

.PHONY: all init clean

all: clean 
	@make $(MAKE_FLAGS) --directory=./init
	@mv ./init/init.elf $(DIR_ROOT)/img-content/bin/init.elf
#	@mv ./init/desktop.elf $(DIR_ROOT)/img-content/bin/desktop.elf

clean:
	@make clean $(MAKE_FLAGS) --directory=./init
#	@make clean $(MAKE_FLAGS) --directory=./desktop