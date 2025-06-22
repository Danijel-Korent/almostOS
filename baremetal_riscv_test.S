.section .rodata
message:
    .string "Hello, World!\n"

.section .text
.globl _start
_start:
    # Set up stack pointer (we dont need SP yet)
    # li sp, 0x80000000

    # Load the UART address
    li t1, 0x10000000

    # Load the address of the message into t0 using absolute addressing
    lui t0, %hi(message)
    addi t0, t0, %lo(message)

print_loop:
    # Load a byte from the message
    lb t2, (t0)
    beqz t2, end_print      # If we read a null-terminator, exit the loop

    # Write the byte to UART reg
    sb t2, (t1)

    # Increment message pointer
    addi t0, t0, 1
    j print_loop            # Jump back to the loop

end_print:
    # Infinite loop
    j end_print

# Ctrl+A + x to exit QEMU
