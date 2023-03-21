target remote localhost:1234
set architecture i386:x86-64
file iso_image_content/boot/AlmostOS_kernel.elf
hbreak kernel_c_main
