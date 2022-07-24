SHELL=/bin/bash

#----------------------------------------------------------------------------
#    DIR_ROOT        The root directory of the project
#    LIB_NAME        The name of the library produced by this makefile
#    POST_BUILD_RULE The optional rule that runs after the lib is built
#	 LIB_DEPS		 The paths to the dependencies of this project
#	 PREPROC		 List of prepocessor definitions
#
#	 CRT0			 The source file for crt0.asm # TODO: remove
#
#	 RUNNABLE		 The path of the binary file if wanted
#	 LD_FILE		 The path of the linker file to link the runnable
#----------------------------------------------------------------------------

# Recursively find files from the current working directory that match param1.
findfiles	= $(patsubst ./%,%,$(shell find . -name $(1)))

# Calculate the list of output directories required by one or more files. The
# base output directory is given in param1, and the list of files is given in
# param2.
outputdirs	= $(addprefix $(dir $(1)/), $(sort $(dir $(2))))

#filter out param2 from param1
filter_out		= $(shell IFS=' ' read -r -a array <<< "$(1)"; echo "$${array[@]/$(2)}")

DIR_LIB_BUILD	:= $(DIR_OBJ)/$(LIB_NAME)
DIR_LIB_OUTPUT	:= $(DIR_BIN)/$(LIB_NAME)

ASM_FILES		:= $(call findfiles,'*.asm')

# TODO: remove this (sort out inside bootloader)
ASM_FILES		:= $(call filter_out,$(ASM_FILES),$(CRT0))
CRT0_OBJ		:= $(if $(CRT0),$(DIR_LIB_BUILD)/${CRT0}.o,)

C_FILES			:= $(call findfiles,'*.c')
CODE_FILES		:= $(ASM_FILES) $(C_FILES)

OBJ_FILES_ASM	:= $(ASM_FILES:%.asm=$(DIR_LIB_BUILD)/%_asm.o)
OBJ_FILES_C		:= $(C_FILES:%.c=$(DIR_LIB_BUILD)/%.o)
OBJ_FILES		:= $(OBJ_FILES_ASM) $(OBJ_FILES_C)

LIB_FILE		:= $(DIR_LIB_BUILD)/$(LIB_NAME).a

TAG	:= $(BLUE)[$(LIB_NAME)]$(NORMAL)

.PHONY: all mkdir link crt

all: mkdir link $(POST_BUILD_RULE)
	@echo "$(TAG) $(SUCCESS)"

mkdir:
	@mkdir -p $(call outputdirs,$(DIR_LIB_BUILD),$(CODE_FILES))
ifdef RUNNABLE
	@mkdir -p $(DIR_LIB_OUTPUT)
endif

link: crt $(OBJ_FILES)
ifdef RUNNABLE
	@echo "$(TAG) Linking $(notdir $(RUNNABLE))"
	@$(LD) $(LDFLAGS) -T $(LD_FILE) -o $(RUNNABLE) $(CRT0_OBJ) $(OBJ_FILES) --start-group $(LIB_DEPS) --end-group
	@chmod a-x $(RUNNABLE)
else
	@echo "$(TAG) Archiving $(notdir $(LIB_FILE))"
	@rm -f $(LIB_FILE)
	@ar cqs $(LIB_FILE) $(CRT0_OBJ) $(OBJ_FILES)
endif

crt:
ifdef CRT0
	@echo "$(TAG) Assembling ${CRT0}"
	@$(AS) $(ASFLAGS) ${CRT0} -o $(CRT0_OBJ)
endif

$(OBJ_FILES_ASM): $(DIR_LIB_BUILD)/%_asm.o: %.asm
	@echo "$(TAG) Assembling $<"
	@$(AS) $(ASFLAGS) $(PREPROC) $< -o $@

$(OBJ_FILES_C): $(DIR_LIB_BUILD)/%.o: %.c
	@echo "$(TAG) Compiling $<"
	@$(CC) $(CCFLAGS) $(PREPROC) -c $< -o $@