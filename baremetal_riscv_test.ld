/* Simple linker script for the baremetal RISC-V test */
OUTPUT_ARCH( "riscv" )
ENTRY( _start )

SECTIONS
{
  /* Start at 0x80000000, which is where QEMU virt machine loads the kernel */
  . = 0x80000000;

  .text : {
    *(.text)
  }

  .rodata : {
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }
} 
