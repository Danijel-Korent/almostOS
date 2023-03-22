# For this Makefile to work, install following packages:
#
# sudo apt install nasm
# sudo apt install build-essential

# For Qemu
# sudo apt install qemu-system

# For Bochs
# sudo apt install bochs
# sudo apt install bochs-sdl
# sudo apt install bochsbios
# sudo apt install genisoimage

# TODO: Replace AlmostOS.iso with variable
# TODO: Replace AlmostOS_kernel.elf with variable


C = gcc
CFLAGS = -m32 -nostdlib -nostartfiles -g

BUILD_DIR = build

obj_files =  $(addprefix $(BUILD_DIR)/, startup.o instruction_wrappers.o kernel_main.o terminal.o hexdump.o BIOS_Data_Area.o)

.PHONY: all
.PHONY: iso_image
.PHONY: kernel
.PHONY: qemu
.PHONY: bochs
.PHONY: gdb_server
.PHONY: clean


##### Compilation rules #####
all: iso_image

iso_image: kernel
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A AlmostOS -input-charset utf8 -boot-info-table -o AlmostOS.iso iso_image_content

kernel: $(obj_files)
	ld -T kernel.ld -melf_i386 $(obj_files) -o iso_image_content/boot/AlmostOS_kernel.elf -Map=memory.map

# Define a pattern rule that compiles every .asm file into an .o file
$(BUILD_DIR)/%.o : %.asm
	nasm -f elf32 $< -o $@

# Define a pattern rule that compiles every .c file into an .o file
$(BUILD_DIR)/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@


##### Commands #####
qemu: all
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -serial stdio

bochs: all
	bochs -f bochs_configuration.txt -q

gdb_server: all
	qemu-system-x86_64 -m 64 -cdrom AlmostOS.iso -boot d -s -S

clean:
	rm $(BUILD_DIR)/*
	rm *.iso
	rm iso_image_content/boot/*.elf
