set architecture i386:x86-64
set disassembly-flavor intel
target remote localhost:1234
file iso_image_content/boot/AlmostOS_kernel.elf
hbreak kernel_c_main
