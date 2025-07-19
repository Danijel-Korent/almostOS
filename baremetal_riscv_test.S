
.section .bss  # standard name of the C memory segment for uninitialized data
    .align 4

    # Reserve memory for kernel stack
    STACK_MEM_END:
        .space 32768
    STACK_MEM_START: # stack grows from higher address to lower, so this is the beginning of the stack


.section .rodata
message:
    .string "Hello, World! \n\n\n"

.section .text
.globl _start
_start:
    # Set up stack pointer (we dont need SP yet)
    la sp, STACK_MEM_START

    # Load the UART address
    li t1, 0x10000000

    # Load the address of the message into t0 using absolute addressing
    lui t0, %hi(message)
    addi t0, t0, %lo(message)

print_loop:
    # Load a byte from the message
    lb t2, (t0)
    beqz t2, c_main      # If we read a null-terminator, exit the loop

    # Write the byte to UART reg
    sb t2, (t1)

    # Increment message pointer
    addi t0, t0, 1
    j print_loop            # Jump back to the loop


c_main:
    j c_test_main

infinite_loop:
    j infinite_loop

# Ctrl+A + x to exit QEMU
