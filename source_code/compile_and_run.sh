
# TODO: Will be replaced with make after code get big enough to be splitted into multiple files

# For this script to work, install following packages:
#
# sudo apt update
# sudo apt install nasm
# sudo apt install build-essential
# sudo apt install bochs
# sudo apt install bochs-sdl
# sudo apt install bochsbios
# sudo apt install genisoimage



# Clean everything before compiling
rm *.iso &> /dev/null
rm *.o   &> /dev/null
rm iso_image_content/boot/*.elf &> /dev/null

# Compile OS startup code in assembly
nasm -f elf32 startup.asm
if [ $? -ne 0 ]; then
    exit
fi

# Compile the C code
gcc -m32 -nostdlib -nostartfiles -c kernel_main.c    -o kernel_main.o       #-Wall
gcc -m32 -nostdlib -nostartfiles -c terminal.c       -o terminal.o          #-Wall
gcc -m32 -nostdlib -nostartfiles -c hexdump.c        -o hexdump.o           #-Wall
gcc -m32 -nostdlib -nostartfiles -c BIOS_Data_Area.c -o BIOS_Data_Area.o    # TODO: Convert into a Makefile!
if [ $? -ne 0 ]; then
    exit
fi

# Link compiled objects into ELF binary using custom linker script "kernel.ld"
ld -T kernel.ld -melf_i386 startup.o kernel_main.o terminal.o hexdump.o BIOS_Data_Area.o -o iso_image_content/boot/AlmostOS_kernel.elf
if [ $? -ne 0 ]; then
    exit
fi

# Generate CD-ROM ISO image containing content of the folder "iso_image_content" and GRUB bootloder with its config in
# file "iso_image_content/boot/grub/menu.lst". In menu.lst. In that file "AlmostOS_kernel.elf" is set as bootable image
# BTW: El Torito is the name of the standard for booting from CD-ROM (https://en.wikipedia.org/wiki/ISO_9660#El_Torito)
genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A AlmostOS -input-charset utf8 -quiet -boot-info-table -o AlmostOS.iso iso_image_content
if [ $? -ne 0 ]; then
    exit
fi

# Run a Bochs PC emulator with custom configuration (configuration sets AlmostOS.iso into CD-ROM of emulated PC)
bochs -f bochs_configuration.txt -q

# I guess I really hate these artifacts
rm *.iso &> /dev/null
rm *.o   &> /dev/null
rm iso_image_content/boot/*.elf &> /dev/null
