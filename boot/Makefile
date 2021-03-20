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

	@echo "$(TAG) Assembling boot.asm"
	@$(AS) $(ASFLAGS) -Wno-all src/boot.asm -o $(DIR_TARGET)/boot.bin

	@echo "$(TAG) Assembling mbr.asm"
	@$(AS) $(ASFLAGS) -Wno-all src/mbr.asm -o $(DIR_TARGET)/mbr.bin

	@echo "$(TAG) Assembling loader.asm"
	#@$(AS) $(ASFLAGS) -Wno-all src/loader.asm -o $(DIR_TARGET)/loader.bin
	@$(AS) -f elf64 -Wno-all src/loader.asm -o $(DIR_OBJ)/boot/loader.o
	@$(LD) -nostdlib -o $(DIR_TARGET)/loader.bin $(DIR_OBJ)/boot/loader.o \
		--oformat=binary --entry=load -Ttext=0x7e00

	@echo "$(TAG) $(SUCCESS)"