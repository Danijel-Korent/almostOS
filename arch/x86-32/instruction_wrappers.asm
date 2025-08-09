; *** x86 INSTRUCTION WRAPPER FUNCTIONS ***

; By 'cdecl' calling convention, all arguments of a function are pushed onto the stack,
; and return value from a function is passed by EAX register. Arguments are pushed to the stack
; in left-to-right order, so first argument is pushed last on the stack and therefore closest to the stack pointer.

; Registers EAX, ECX, and EDX are caller-saved, and the rest are callee-saved.
; Instruction "push" always pushes 4 bytes in 32-bit code, or 8 in 64-bit code

; x86's stack grows towards lower addresses, so first argument (last on stack) have lowest address and last arg highest address.

; GDB commands:
;   disass /r kernel_c_main
;   info frame
;   layout next
;   nexti
;   x/20x $esp


global test_func
global read_byte_from_IO_port
global write_byte_to_IO_port
global get_timestamp
global halt_cpu


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

global get_GDT_table

; void* get_GDT_table(void)
get_GDT_table
    sgdt [store_gdtr]
    mov eax, store_gdtr
    ret


section .data
store_gdtr:
    dw 0    ; limit (2b)
    dd 0    ; base  (4b)

