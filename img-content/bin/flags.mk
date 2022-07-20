CCFLAGS	:= -std=gnu11 $(DIRS_INCLUDE) -g \
			-D BONSOS \
			-masm=intel \
			-mcmodel=large \
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough \
			-Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context