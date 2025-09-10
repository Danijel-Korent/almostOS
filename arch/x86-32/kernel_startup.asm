; Sets Multiboot signature and defines function start_kernel() which is in kernel.ld linker script "kernel.ld" set as
; the entry point for the compiled artifact. The start_kernel() just outputs string "hello_msg", sets stack pointer and
; then jumps to the first C code function: kernel_c_main()

; INFO:
;  - https://en.wikibooks.org/wiki/X86_Assembly/NASM_Syntax
;  - http://kernfunny.org/x86/ (x86 Instruction Set Reference)
;

; https://en.wikipedia.org/wiki/Multiboot_specification
MULITBOOT_FLAGS        equ 0x0
MULITBOOT_MAGIC_NUMBER equ 0x1BADB002

VGA_RAM_ADDRESS        equ 0x000B8000 ;Video memory address for text mode - 32kb total
                                      ;Here we assume that VGA is set to the video mode 03 (80x25) by the BIOS or bootloader
                                      ;http://www.scs.stanford.edu/17wi-cs140/pintos/specs/freevga/vga/vgamem.htm


; Export these as global symbols:
global start_kernel  ; GRUB will jump into this function (specified in the linker script "kernel.ld")
global STACK_MEM_START
global STACK_MEM_END

; declarations for outside functions:
extern kernel_c_main ; entry point for C code


; I already forgot why I had put the stack inside the .bss segment. Maybe so that Grub will zero-initiate it
; on image load?
section .bss  ; standard name of the C memory segment for uninitialized data
    align 4

    ; Reserve memory for kernel stack
    STACK_MEM_END:
        resb 1048576    ; 1MB ought to be enough for everyone
    STACK_MEM_START:    ; on x86 stack grows from higher address to lower, so this is the beginning of the stack


; Had to create separate mem. segment for multiboot because position of the header would move as the build order changes
; and at one point it moved out of the first 8192 bytes of the image, making the image unbootable
section .multiboot

    ; Per multiboot specification. Without this GRUB will not recognize this binary image as bootable
    ; https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
    align 4
    dd MULITBOOT_MAGIC_NUMBER
    dd MULITBOOT_FLAGS
    dd -(MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS) ; multiboot checksum: this calculated number + MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS must equal zero


section .text ; standard name of the C memory segment for code
hello_msg:
    db "Starting AlmostOS kernel...", 0

start_kernel:
    mov eax, 0xFEEDBEEF ;a classic

    mov esi, hello_msg
    mov edi, VGA_RAM_ADDRESS

    ;int 0x1

.print_hello:         ;shit just got real
    mov cl, [esi]
    jcxz .start_c_main         ;stop printing on null-terminator

    mov byte [edi], cl
    inc edi

    mov byte [edi], 0x02 ; Cool looking green with the black backgroud
    inc edi

    inc esi
    jmp .print_hello

.start_c_main:
    ; Set up stack pointer for C code execution
    ; If we set SP to STACK_MEM_START (without -64), the return address would be outside of stack memory.
    ; The kernel doesn't care about the return address as there is no return from C main(), but the GDB
    ; tries read it to reconstruct the back trace, and without -32, the place where return address would be
    ; is acuppied by some variable with some value, and then GDB sometimes construct crazy long backtraces
    ; by reading this random value and interpreting it as function address on the call stack
    ; Also -64 give us some empty space between the stack and the variables outside of stack (return addr is only 32-bit)
    mov ebp, STACK_MEM_START - 64
    mov esp, STACK_MEM_START - 64

    call kernel_c_main ; Call the main/entry function of the C code

.loop:
    ; TODO: print here "Exited kernel C code" or something similar
    jmp .loop ;infinite loop

