rm *.iso &> /dev/null
rm *.o   &> /dev/null
rm iso_image_content/boot/*.elf &> /dev/null

nasm -f elf32 startup.asm
if [ $? -ne 0 ]; then
    exit
fi

gcc -m32 -nostdlib -nostartfiles -c kernel_main.c -o kernel_main.o
if [ $? -ne 0 ]; then
    exit
fi

ld -T kernel.ld -melf_i386 startup.o kernel_main.o -o iso_image_content/boot/AlmostOS_kernel.elf
if [ $? -ne 0 ]; then
    exit
fi

genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A AlmostOS -input-charset utf8 -quiet -boot-info-table -o AlmostOS.iso iso_image_content
if [ $? -ne 0 ]; then
    exit
fi

bochs -f bochs_configuration.txt -q


rm *.iso &> /dev/null
rm *.o   &> /dev/null
rm iso_image_content/boot/*.elf &> /dev/null
