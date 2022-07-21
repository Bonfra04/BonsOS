CCFLAGS	:= -std=gnu11 $(DIRS_INCLUDE) -g \
			-D BONSOS \
			-masm=intel \
			-mcmodel=large \
			-mno-red-zone \ # TODO: remove this (https://wiki.osdev.org/Calling_Conventions##:~:text=Note%202)
			-Wall -Wextra \
			-fplan9-extensions \
			-Wno-misleading-indentation -Wno-parentheses -Wno-implicit-fallthrough \
			-Wno-sign-compare -Wno-address-of-packed-member -Wno-int-in-bool-context