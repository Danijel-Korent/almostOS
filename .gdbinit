file arch/x86-32/iso_image_content/boot/AlmostOS_kernel.elf
set architecture i386
set disassembly-flavor intel
target remote localhost:1234
hbreak start_kernel
hbreak kernel_c_main
hbreak 0
hbreak syscall_test
