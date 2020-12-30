DIR_ROOT	:= ..

include $(DIR_ROOT)/scripts/config.mk

DIR_TARGET	:= $(DIR_BIN)/boot
ASFLAGS 	:= -f bin

ASM_FILES	:= $(wildcard *.asm)
BIN_FILES	:= $(ASM_FILES:%.asm=$(DIR_TARGET)/%.bin)

TAG 		:= $(BLUE)[boot]$(NORMAL)

.PHONY: all mkdir

all: mkdir $(BIN_FILES)
	@echo "$(TAG) $(SUCCESS)"

mkdir:
	@mkdir -p $(DIR_TARGET)

$(BIN_FILES): $(DIR_TARGET)/%.bin: %.asm
	@echo "$(TAG) Assembling $<"
	@$(AS) $(ASFLAGS) -Wno-all $< -o $@
