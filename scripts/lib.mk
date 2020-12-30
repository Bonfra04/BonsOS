#----------------------------------------------------------------------------
#    DIR_ROOT        The root directory of the project
#    LIB_NAME        The name of the library produced by this makefile
#    POST_LIB_RULE   The optional rule that runs after the lib is built
#----------------------------------------------------------------------------

include $(DIR_ROOT)/scripts/config.mk

# Recursively find files from the current working directory that match param1.
findfiles	= $(patsubst ./%,%,$(shell find . -name $(1)))

# Calculate the list of output directories required by one or more files. The
# base output directory is given in param1, and the list of files is given in
# param2.
outputdirs	= $(addprefix $(dir $(1)/), $(sort $(dir $(2))))

# Calculate path of lib file from lib name
libfile		= $(join $(1:%=$(DIR_OBJ)/%), $(1:%=/%.a))

DIR_LIB_BUILD	:= $(DIR_OBJ)/$(LIB_NAME)

ASM_FILES		:= $(call findfiles,'*.asm')
C_FILES			:= $(call findfiles,'*.c')
CODE_FILES		:= $(ASM_FILES) $(C_FILES)

OBJ_FILES_ASM	:= $(ASM_FILES:%.asm=$(DIR_LIB_BUILD)/%_asm.o)
OBJ_FILES_C		:= $(C_FILES:%.c=$(DIR_LIB_BUILD)/%.o)
OBJ_FILES		:= $(OBJ_FILES_C) $(OBJ_FILES_ASM)

LIB_FILE		:= $(DIR_LIB_BUILD)/$(LIB_NAME).a

TAG	:= $(BLUE)[$(LIB_NAME)]$(NORMAL)

.PHONY: all mkdir resolve_deps

all: mkdir resolve_deps $(LIB_FILE) $(POST_LIB_RULE)
	@echo "$(TAG) $(SUCCESS)"

mkdir:
	@mkdir -p $(call outputdirs,$(DIR_LIB_BUILD),$(CODE_FILES))

resolve_deps:

$(LIB_FILE): $(OBJ_FILES)
	@echo "$(TAG) Archiving $(notdir $@)"
	@rm -f $@
	@ar cqs $@ $(OBJ_FILES)

$(OBJ_FILES_ASM): $(DIR_LIB_BUILD)/%_asm.o: %.asm
	@echo "$(TAG) Assembling $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJ_FILES_C): $(DIR_LIB_BUILD)/%.o: %.c
	@echo "$(TAG) Compiling $<"
	@$(CC) $(CCFLAGS) -c $< -o $@