
; INFO:
;  - https://en.wikibooks.org/wiki/X86_Assembly/NASM_Syntax
;  - https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
;

MULITBOOT_FLAGS equ 0x0

global start_kernel ;GRUB will jump into this function

section .text ; standard name of the C memory segment for code

    ; per multiboot specification. Without this GRUB will not recognite this binary image as bootable
    align 4
    dd 0x1BADB002 ;multiboot magic number
    dd MULITBOOT_FLAGS
    dd -(0x1BADB002 + MULITBOOT_FLAGS) ; when be zero when magic_number+multiboot flags are summed with it


start_kernel:
    mov eax, 0xFEEDBEEF ;a classic

.loop:
    jmp .loop ;infinite loop
