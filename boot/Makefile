DIR_ROOT	:= ..

include $(DIR_ROOT)/scripts/config.mk

findfiles	= $(patsubst ./src/%,%,$(shell find . -name $(1)))
outputdirs	= $(addprefix $(dir $(1)/), $(sort $(dir $(2))))

DIR_TARGET	:= $(DIR_BIN)/boot
ASFLAGS 	:= -f bin

TAG 		:= $(BLUE)[boot]$(NORMAL)

.PHONY: all mkdir

all:
	@mkdir -p $(DIR_TARGET)
	@mkdir -p $(DIR_OBJ)/boot

	@echo "$(TAG) Assembling mbr.asm"
	@$(AS) $(ASFLAGS) -Wno-all src/mbr.asm -o $(DIR_TARGET)/mbr.bin

	@echo "$(TAG) Assembling vbr.asm"
	@$(AS) $(ASFLAGS) -Wno-all src/vbr.asm -o $(DIR_TARGET)/vbr.bin

	@echo "$(TAG) Assembling loader.asm"
	@$(AS) -f elf64 -Wno-all src/loader.asm -o $(DIR_OBJ)/boot/loader.o
	@$(LD) -nostdlib -o $(DIR_TARGET)/loader.bin $(DIR_OBJ)/boot/loader.o -T boot.ld
	
	@echo "$(TAG) $(SUCCESS)"