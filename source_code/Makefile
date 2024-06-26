# For this Makefile to work, install following packages:
#
# sudo apt install build-essential nasm genisoimage

# For Qemu
# sudo apt install qemu-system

# For Bochs
# sudo apt install bochs bochs-sdl bochsbios

# TODO: Replace AlmostOS.iso with a variable
# TODO: Replace AlmostOS_kernel.elf with a variable


# Compiler, assembler and flags
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -nostartfiles -isystem system_headers -I fs/tinyfs/api -g  # "nostartfiles" is probably unnecessary since I've defined entry point in the .ld

AS = nasm
ASFLAGS = -f elf32

# Source and compilation artifacts directories
SRC_DIR = .
SKIP_DIR = fs
OBJ_DIR = ./build

# Include directories
INCLUDE_DIRS := . drivers utils memory shell
INCLUDE_FLAGS := $(addprefix -I, $(INCLUDE_DIRS))

###### TEMP TinyFS ######
# Temp until I decide how I will include the TinyFS to this Makefile
TINY_FS_DIR = ./fs/tinyfs
TINY_FS_SRCS = $(TINY_FS_DIR)/code/cluster_operations.c $(TINY_FS_DIR)/code/disk_image.c $(TINY_FS_DIR)/code/FAT_cluster.c $(TINY_FS_DIR)/code/fs_operations.c $(TINY_FS_DIR)/code/rootdir_cluster.c $(TINY_FS_DIR)/code/util.c
TINY_FS_OBJS = $(TINY_FS_SRCS:.c=.o) # Define the object files based on the source files
###### TEMP TinyFS ######

# Finding all .c and .asm files in subdirectories
SRCS_C   := $(shell find $(SRC_DIR) -type d -name $(SKIP_DIR) -prune -o -type f -name "*.c" -print) # I don't like this anymore
SRCS_ASM := $(shell find $(SRC_DIR) -type d -name $(SKIP_DIR) -prune -o -type f -name "*.asm" -print)

# Replacing .c with .o and .asm with .o and adding the object directory prefix
OBJS_C   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(SRCS_C))
OBJS_ASM := $(patsubst $(SRC_DIR)/%.asm, $(OBJ_DIR)/%.o, $(SRCS_ASM))

obj_files := $(OBJS_C) $(OBJS_ASM) $(TINY_FS_OBJS)


.PHONY: all
.PHONY: iso_image
.PHONY: kernel
.PHONY: qemu
.PHONY: bochs
.PHONY: gdb_server
.PHONY: clean


##### Compilation rules #####
all: qemu

iso_image: kernel
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A AlmostOS -input-charset utf8 -boot-info-table -o AlmostOS.iso iso_image_content

kernel: $(obj_files)
	ld -T kernel.ld -melf_i386 $(obj_files) -o iso_image_content/boot/AlmostOS_kernel.elf -Map=memory.map

# Define a pattern rule that compiles every .asm file into an .o file
$(OBJ_DIR)/%.o : %.asm
	@mkdir -p $(@D)  # Create the directory if it does not exist
	$(AS) $(ASFLAGS) $< -o $@

# Define a pattern rule that compiles every .c file into an .o file
$(OBJ_DIR)/%.o : %.c
	@mkdir -p $(@D)  # Create the directory if it does not exist
	$(CC) -c $(CFLAGS) $(INCLUDE_FLAGS) $< -o $@


##### Commands #####
qemu: all
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -serial stdio

bochs: all
	bochs -f bochs_configuration.txt -q

gdb_server: all
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -s -S

clean:
	-rm -r $(OBJ_DIR)/*
	-rm *.iso
	-rm iso_image_content/boot/*.elf
