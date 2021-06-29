DIR_ROOT := .

include $(DIR_ROOT)/scripts/config.mk

.PHONY: all build bootloader kernel libk libc programs libusr mkdir clean image

all: build

build: mkdir bootloader kernel programs image
	@echo "Done"

bootloader:
	@make $(MAKE_FLAGS) --directory=$(DIR_BOOT)

kernel: libk libc libusr
	@make $(MAKE_FLAGS) --directory=$(DIR_KERNEL)

libk:
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBK)

libc:
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBC) kernel=1
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBC)

programs:
	@make $(MAKE_FLAGS) --directory=$(DIR_PROGRAMS)

libusr:
	@make $(MAKE_FLAGS) --directory=$(DIR_LIBUSR)

mkdir:
	@mkdir -p bin
	@mkdir -p bin/img

clean:
	@rm -fr bin
	@rm -fr bin-int
	@rm -f BonsOS.img
	
image:
	@echo "Generating image..."
	@bash ./scripts/create_image.sh > /dev/null 2>&1