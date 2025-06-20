.section .data
message:
    .string "Hello, World!\n"

.section .text
.globl _start
_start:
    # Load file descriptor (1 for stdout)
    li a0, 1

    # Load the address of the message
    la a1, message

    # Load the length of the message
    li a2, 13

    # Load the Linux RISC-V write syscall number
    li a7, 64

    # Make the system call
    ecall

    # Exit the program
    li a0, 0          # Set return code to 0
    li a7, 93         # Linux RISC-V exit syscall number
    ecall
