# For this Makefile to work, install following packages:
#
# sudo apt install build-essential nasm genisoimage

# For Qemu
# sudo apt install qemu-system

# TODO: Replace AlmostOS.iso with a variable
# TODO: Replace AlmostOS_kernel.elf with a variable


# Compiler, assembler and flags
CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -nostartfiles -isystem system_headers -I fs/tinyfs/api -I arch/x86-32 -g  # "nostartfiles" is probably unnecessary since I've defined entry point in the .ld

AS = nasm
ASFLAGS = -f elf32

# Source and compilation artifacts directories
SRC_DIR = .
ARCH_X86_32_DIR = ./arch/x86-32/
ARCH_RISC_V_DIR = ./arch/risc-v/
SKIP_DIR_1 = fs
SKIP_DIR_2 = arch
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
SRCS_C_COMMON   := $(shell find $(SRC_DIR) -type d \( -name $(SKIP_DIR_1) -o -name $(SKIP_DIR_2) \) -prune -o -type f -name "*.c" -print) # I don't like this anymore

# Arch specific
SRCS_C_X86_32   := $(shell find $(ARCH_X86_32_DIR) -type d -name $(SKIP_DIR_1) -prune -o -type f -name "*.c" -print)
SRCS_ASM_X86_32 := $(shell find $(ARCH_X86_32_DIR) -type d -name $(SKIP_DIR_1) -prune -o -type f -name "*.asm" -print)

# Currently hardcoded to x86, but will later add rules and env var to chose between x86 and RISC-V 
SRCS_C   := $(SRCS_C_COMMON) $(SRCS_C_X86_32)
SRCS_ASM := $(SRCS_ASM_X86_32)

# Replacing .c/.asm with .o, and adding the object directory prefix
OBJS_C   := $(patsubst $(SRC_DIR)/%.c,   $(OBJ_DIR)/%.o, $(SRCS_C))
OBJS_ASM := $(patsubst $(SRC_DIR)/%.asm, $(OBJ_DIR)/%.o, $(SRCS_ASM))

obj_files := $(OBJS_C) $(OBJS_ASM) $(TINY_FS_OBJS)


.PHONY: all
.PHONY: iso_image
.PHONY: kernel
.PHONY: qemu
.PHONY: gdb_server
.PHONY: clean
.PHONY: linux_risc_v_test
.PHONY: baremetal_risc_v_test


##### Compilation rules #####
all: qemu

iso_image: kernel
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A AlmostOS -input-charset utf8 -boot-info-table -o AlmostOS.iso arch/x86-32/iso_image_content

kernel: $(obj_files)
	ld -T kernel.ld -melf_i386 $(obj_files) -o arch/x86-32/iso_image_content/boot/AlmostOS_kernel.elf -Map=memory.map

# Define a pattern rule that compiles every .asm file into an .o file
$(OBJ_DIR)/%.o : %.asm
	@mkdir -p $(@D)  # Create the directory if it does not exist
	$(AS) $(ASFLAGS) $< -o $@

# Define a pattern rule that compiles every .c file into an .o file
$(OBJ_DIR)/%.o : %.c
	@mkdir -p $(@D)  # Create the directory if it does not exist
	$(CC) -c $(CFLAGS) $(INCLUDE_FLAGS) $< -o $@


##### Commands #####

# TODO:
#   In theory I could run the kernel with just "qemu-system-x86_64 -kernel arch/x86-32/iso_image_content/boot/AlmostOS_kernel.elf -serial stdio"
#   But for that I would need to implemented some x86 specific setup and init already done by grub (like setting segment regs, VGA mode),
#   and I don't want to waste my time on this right now
qemu: iso_image
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -serial stdio

gdb_server: iso_image
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -s -S

clean:
	rm -rf ./$(OBJ_DIR)/*
	rm -f fs/tinyfs/code/*.o
	rm -f *.iso
	rm -f *.bin
	rm -f *.dis
	rm -f *.map
	rm -f arch/x86-32/iso_image_content/boot/*.elf


linux_risc_v_test:
	riscv64-unknown-elf-as -march=rv32i -o linux_riscv_test.o user-space/risc-v/linux_riscv_test.S
	riscv64-unknown-elf-ld -m elf32lriscv -o linux_riscv_test.elf linux_riscv_test.o
	riscv64-unknown-elf-objdump -d -M numeric linux_riscv_test.elf > linux_riscv_test.dis
	qemu-riscv32 ./linux_riscv_test.elf # -strace

baremetal_risc_v_test:
	riscv64-unknown-elf-as -march=rv32i -o baremetal_riscv_test.o baremetal_riscv_test.S
	riscv64-unknown-elf-ld -m elf32lriscv -T baremetal_riscv_test.ld -o baremetal_riscv_test.elf baremetal_riscv_test.o
	riscv64-unknown-elf-objdump -d baremetal_riscv_test.elf > baremetal_riscv_test.dis # -M numeric
	riscv64-unknown-elf-objcopy -O binary baremetal_riscv_test.elf baremetal_riscv_test.bin
	qemu-system-riscv32 -nographic -machine virt -bios none -kernel baremetal_riscv_test.bin # -d guest_errors
