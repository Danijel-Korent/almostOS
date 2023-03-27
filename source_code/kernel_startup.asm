


    ; NEXT TODO: 
    ;
    ;    - Commit build files somewhere, maybe put:
    ;       - bootloader image to ./bootloader_image or ./build/bootloader_image
    ;       - makefiles and emulator configuration to ./build
    ;       - OS image to ./build/os_image
    ;
    ;    - Prepare C runtime
    ;
    ;        - Reserve the memory space for stack
    ;           - since we are using and GRUB's ELF loader to load memory segments into adress space
    ;             the safest way to do that is to place it to the .data or .bss memory segment
    ;             The .bss is better choice since its content is not packed into the ELF binary
    ;
    ;        - Clear all .bss memory section to zeros. (done by bootloader??)
    ;            - Here we will need to do some tricks, since we will know the exact size of the .bss segment only after linking.
    ;
    ;        - Jump to C program entry
    ;            - lets call it _kernel_c_main
    ;
    ;
    ;    - Implement driver for text output
    ;
    ;    - Implement driver for the PIC
    ;    - Implement driver for text input
    ;
    ;    - Add the FAT file reader, shell and some FS image from the "FAT-filesystem-reader" git project
    ;        - So that OS at least looks like it could do something useful hehe 


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

; Export these as global functions
global start_kernel  ; GRUB will jump into this function (specified in the linker script "kernel.ld")

extern kernel_c_main ; entry point for C code


section .bss  ; standard name of the C memory segment for uninitialized data
    align 4

    ; Reserve memory for kernel stack
    STACK_MEM_END:
        resb 32768
    STACK_MEM_START: ; on x86 stack grows from higher address to lower, so this is the beginning of the stack


section .text ; standard name of the C memory segment for code

    ; Per multiboot specification. Without this GRUB will not recognize this binary image as bootable
    ; https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
    align 4
    dd MULITBOOT_MAGIC_NUMBER
    dd MULITBOOT_FLAGS
    dd -(MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS) ; multiboot checksum: this calculated number + MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS must equal zero


hello_msg:
    db "Starting AlmostOS kernel...", 0

start_kernel:
    mov eax, 0xFEEDBEEF ;a classic

    mov esi, hello_msg
    mov edi, VGA_RAM_ADDRESS


.print_hello:         ;shit just got real
    mov cl, [esi]
    jcxz .start_c_main         ;stop printing on null-terminator

    mov byte [edi], cl
    inc edi

    mov byte [edi], 0x02 ; Cool looking green with the black backgroud
    inc edi

    inc esi
    jmp .print_hello

    ; set up stack pointer for C code execution
    mov esp, STACK_MEM_START

.start_c_main:
    call kernel_c_main ; Call the main/entry function of the C code

.loop:
    ; TODO: print here "exiting kernel" or something similar
    jmp .loop ;infinite loop
