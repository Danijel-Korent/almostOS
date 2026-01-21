file arch/x86-32/iso_image_content/boot/AlmostOS_kernel.elf
set architecture i386
set disassembly-flavor intel
target remote localhost:1234
hbreak start_kernel
hbreak kernel_c_main
hbreak 0
hbreak syscall_test
hbreak test_INT_HANDLER_00_DIVIDE_ERROR
hbreak syscall_entry_point_asm
hbreak INT_80_test
hbreak *0x0010594f

