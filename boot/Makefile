DIR_ROOT	:= ..

include $(DIR_ROOT)/scripts/config.mk

DIR_TARGET	:= $(DIR_BIN)/boot
ASFLAGS 	:= -f bin
CCFLAGS	 := -std=gnu11 -Qn -g \
			-m32 -mno-red-zone -mno-mmx -mfpmath=sse -masm=intel \
			-ffreestanding -fno-asynchronous-unwind-tables \
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough -Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context

TAG 		:= $(BLUE)[boot]$(NORMAL)

.PHONY: all mkdir stage1 stage2

all: mkdir stage1 stage2
	@echo "$(TAG) $(SUCCESS)"

stage2:
	@make $(MAKE_FLAGS) --directory=$(DIR_BOOT)/stage2

stage1:
	@echo "$(TAG) Assembling mbr.asm"
	@$(AS) $(ASFLAGS) -Wno-all stage1/mbr.asm -o $(DIR_TARGET)/mbr.bin
	@echo "$(TAG) Assembling vbr.asm"
	@$(AS) $(ASFLAGS) -Wno-all stage1/vbr.asm -o $(DIR_TARGET)/vbr.bin

mkdir:
	@mkdir -p $(DIR_TARGET)
	@mkdir -p $(DIR_OBJ)/boot