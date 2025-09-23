; *** x86 INSTRUCTION WRAPPER FUNCTIONS ***

; By 'cdecl' calling convention
;   - all arguments of a function are pushed onto the stack
;       - pushed in right-to-left order, so first argument is pushed last on the stack and therefore closest to the stack pointer
;   - return value --> passed by EAX register

; Registers EAX, ECX, and EDX are caller-saved, and the rest are callee-saved.
; Instruction "push" always pushes 4 bytes in 32-bit code, or 8 in 64-bit code

; x86's stack grows towards lower addresses, so first argument (last on stack) have lowest address and last arg highest address.

; GDB commands:
;   disass /r kernel_c_main
;   info frame
;   layout next
;   nexti
;   x/20x $esp

; TODO: Check if any if this functions modifes registers that should be preserved per cdecl convention
;       Only EAX, ECX, and EDX are caller-saved

; TODO: Implement wrapper for CPUID

global test_func
global read_byte_from_IO_port
global write_byte_to_IO_port
global get_timestamp
global halt_cpu
global get_reg_CS
global get_reg_DS
global get_reg_SS
global get_reg_ES
global get_reg_FS
global get_reg_GS
global random_test

; void random_test(void);
random_test:
    int 0x80
    ; int 0x0D
    ; int 0x00
    ret

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


; https://www.felixcloutier.com/x86/out
; PROTO: void write_byte_to_IO_port( uint16_t port_address, uint8_t data)
write_byte_to_IO_port:
    mov dx, [ESP+4] ; IO port address
    mov al, [ESP+8] ; data
    out dx, al
    ret


; PROTO: int get_timestamp(void)
get_timestamp:
    RDTSC   ; Timestamp is loaded into EDX:EAX. We just ignore high bits in EDX for now
    ret


halt_cpu:
    hlt
    ret


get_reg_CS:
    mov eax, cs
    ret

get_reg_DS:
    mov eax, ds
    ret

get_reg_SS:
    mov eax, ss
    ret

get_reg_ES:
    mov eax, es
    ret

get_reg_FS:
    mov eax, fs
    ret

get_reg_GS:
    mov eax, gs
    ret


global get_GDT_table_location

; void* get_GDT_table_location(void)
get_GDT_table_location:
    sgdt [store_gdtr]
    mov eax, store_gdtr
    ret


; void* get_IDT_table_location(void)
global get_IDT_table_location
get_IDT_table_location:
    sidt [store_idt]
    mov eax, store_idt
    ret

; void set_IDT_table_location(void)
global set_IDT_table_location
set_IDT_table_location:
    lidt [store_idt]
    ret

; TODO: Replace this with just passing an argument
section .data
store_gdtr:
    dw 0    ; limit (2b)
    dd 0    ; base  (4b)

store_idt:
    dw 0    ; limit (2b)
    dd 0    ; base  (4b)

