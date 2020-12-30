DIR_ROOT := .

include $(DIR_ROOT)/scripts/config.mk

.PHONY: all build bootloader kernel libk libc mkdir clean

all: build

build: clean mkdir bootloader kernel

bootloader:
	@make $(MAKE_FLAGS) --directory=$(DIR_BOOT)

kernel: libk libc
	@make $(MAKE_FLAGS) --directory=$(DIR_KERNEL)

libk: libc
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBK)

libc:
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBC)

mkdir:
	@mkdir bin

clean:
	@rm -fr bin
	@rm -fr bin-int