


    ; NEXT TODO: 
    ;
    ;    - Commit build files somewhere (maybe put bootloader image to ./bootloader_image or ./build/bootloader_image; makefiles and emulator configuration to ./build; OS image to ./build/os_image)
    ;
    ;    - Prepare C runtime
    ;
	;        - Reserve the memory space for stack
	;           - since we are using and GRUB's ELF loader to load memory segments into adress space
	;             the safest way to do that is to place it to the .data or .bss memory segment
	;             The .bss is better choice since its content is not packed into the ELF binary
	;
	;        - Setup the stack frame
	;
	;        - Clear all .bss memory section to zeros.
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
;  - https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
;  - http://kernfunny.org/x86/ (x86 Instruction Set Reference)
;

MULITBOOT_FLAGS        equ 0x0
MULITBOOT_MAGIC_NUMBER equ 0x1BADB002

global start_kernel ;GRUB will jump into this function

section .text ; standard name of the C memory segment for code

    ; per multiboot specification. Without this GRUB will not recognite this binary image as bootable
    align 4
    dd MULITBOOT_MAGIC_NUMBER
    dd MULITBOOT_FLAGS
    dd -(MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS) ; multiboot checksum: this number + MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS must equal zero

hello_msg:
	db "Starting AlmostOS kernel...", 0

start_kernel:
    mov eax, 0xFEEDBEEF ;a classic

    mov esi, hello_msg
    mov edi, 0x000B8000

.print_hello: 		;shit just got real
    mov cl, [esi]
    jcxz .loop 		;stop printing on null-terminator

    mov byte [edi], cl
    inc edi

    mov byte [edi], 0x02 ; Cool looking green with the black backgroud
    inc edi

    inc esi
    jmp .print_hello


.loop:
    jmp .loop ;infinite loop
