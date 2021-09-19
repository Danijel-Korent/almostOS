


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
;  - https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
;  - http://kernfunny.org/x86/ (x86 Instruction Set Reference)
;

MULITBOOT_FLAGS        equ 0x0
MULITBOOT_MAGIC_NUMBER equ 0x1BADB002
VGA_RAM_ADDRESS        equ 0x000B8000 ;Video memory address for text mode - 32kb total
                                      ;Here we assume that VGA is set to the video mode 03 (80x25) by the BIOS or bootloader
                                      ;http://www.scs.stanford.edu/17wi-cs140/pintos/specs/freevga/vga/vgamem.htm

global start_kernel  ; GRUB will jump into this function (specified by the linker script)
global test_func
global read_byte_from_IO_port

extern kernel_c_main ; entry point for C code


section .bss  ; standard name of the C memory segment for uninitialized data
    align 4

    ; Reserve memory for kernel stack
    STACK_MEM_END:
        resb 32768
    STACK_MEM_START: ; on x86 stack grows from higher address to lower, so this is the beginning of the stack


section .text ; standard name of the C memory segment for code

    ; per multiboot specification. Without this GRUB will not recognize this binary image as bootable
    align 4
    dd MULITBOOT_MAGIC_NUMBER
    dd MULITBOOT_FLAGS
    dd -(MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS) ; multiboot checksum: this number + MULITBOOT_MAGIC_NUMBER + MULITBOOT_FLAGS must equal zero

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


; *** x86 INSTRUCTION WRAPPER FUNCTIONS ***

; By 'cdecl' calling convention, all arguments of a function are pushed onto the stack,
; and return value from a function is passed by EAX register. Arguments are pushed to the stack
; in left-to-right order, so first argument is pushed last on the stack and therefore closest to the stack pointer.
; Registers EAX, ECX, and EDX are caller-saved, and the rest are callee-saved.
;
; x86's stack grows towards lower addresses, so first argument (last on stack) have lowest address and last arg highest address.
; PROTO: int test_func(int base, int multiplier, int adder);
test_func:
    mov eax, [ESP+4]
    mov ecx, [ESP+8]
    mul ecx
    add eax, [ESP+12]
    ret


; https://c9x.me/x86/html/file_module_x86_id_139.html
; PROTO: uint8_t read_byte_from_IO_port( uint16_t port_address)
read_byte_from_IO_port:
    mov dx, [ESP+4] ; move the 'port_address' argument to the DX (16-bit) register
    mov eax, 0      ; Clear whole 32 bits of reg for return value. Is this actually neccessary?
    in al, dx       ; Perform reading of IO port into AL (8-bit) register using DX as port number
    ret


write_byte_to_IO_port:
    ret
