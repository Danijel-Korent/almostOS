extern schedule_in_irq_context

global syscall_entry_point_asm


section .text

; Entry point for int 0x80 Linux syscall interface
; NOTE: This is called in IRQ context when int 0x80 is executed!
syscall_entry_point_asm:
    pushad      ; Push (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) - WARNING: Not available on x86-64 !
    pushfd      ; Push the flags register (EFLAGS)

    ; This registers contains arguments of syscall
    ; We push them to stack to make them accesible as function arguments for a C function syscall_entry_point_C, per cdecl
    push edx  ; arg 3
    push ecx  ; arg 2
    push ebx  ; arg 1
    push eax  ; sycall number

    call syscall_entry_point_C

    pop eax
    pop eax
    pop eax
    pop eax ; I'm layz sometimes. TODO: Just move the stack pointer

    popfd       ; Restore the flags register
    popad       ; Restore all general-purpose registers

    call schedule_in_irq_context ; TODO: everything after this is never called becase of "iret" being executed inside schedule_in_irq_context

    iret



;------------------------------------------------------/ test code for syscall impl. /------------------------------------------------------

global syscall_test
global INT_80_test
global INT_80_test_silent


section .data
    msg_int80_test db 0xa, 0xa,'           >>>> Executed INT_80_test sycall ASM !!! <<< ', 0xa   ; String to print with newline (0xa)
    msg_int80_test_len equ $ - msg_int80_test  ; Calculate length of message

section .text

; void INT_80_test(void);
INT_80_test:
    pushad

    ; System call: write(int fd, const char *buf, size_t count)
    ; eax = 4 (sys_write)
    ; ebx = file descriptor (1 for stdout)
    ; ecx = pointer to message
    ; edx = length of message

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    mov ecx, msg_int80_test
    mov edx, msg_int80_test_len
    int 0x80                       ; make system call using int 0x80

    popad
    ret


section .data
    msg_int80_test_silent db 0xa
    msg_int80_test_silent_len equ $ - msg_int80_test_silent  ; Calculate length of message

section .text

; void INT_80_test_silent(void);
INT_80_test_silent:
    pushad

    ; System call: write(int fd, const char *buf, size_t count)
    ; eax = 4 (sys_write)
    ; ebx = file descriptor (1 for stdout)
    ; ecx = pointer to message
    ; edx = length of message

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    mov ecx, msg_int80_test_silent
    mov edx, msg_int80_test_silent_len
    int 0x80                       ; make system call using int 0x80

    popad
    ret


extern sys_write_test
extern sys_write                ; ssize_t sys_write(int fd, const void *buf, size_t count)
extern syscall_entry_point_C    ; void syscall_entry_point_C(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)


section .data
    msg_sys_test db 0xa, 0xa,'           >>>> CALLED syscall_entry_point ASM !!! <<< ', 0xa   ; String to print with newline (0xa)
    msg_sys_test_len equ $ - msg_sys_test  ; Calculate length of message


section .text
; TODO: For testing only/ This can be removed later
syscall_entry_point_asm_1_sys_write_test:
    pushad      ; Push all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) - WARNING: Not available on x86-64 !
    pushfd      ; Push the flags register (EFLAGS)

    push msg_sys_test_len
    push msg_sys_test
    push 1
    call sys_write
    ;call sys_write_test
    pop eax
    pop eax
    pop eax

    popfd       ; Restore the flags register
    popad       ; Restore all general-purpose registers
    iret


; 32-bit x86 Linux assembly program to print "Hello World"

; A mess with x86 system call instructions in compat mode:
; https://reverseengineering.stackexchange.com/questions/16454/struggling-between-syscall-or-sysenter-windows
; https://stackoverflow.com/questions/77678700/calling-system-api-from-32-bit-processes-under-linux-64-bit/77680771#77680771

; qemu-i386 ./linux_x86-32_test

section .data
    message db 'syscall_test executed', 0xa   ; String to print with newline (0xa)
    message_length equ $ - message  ; Calculate length of message

section .text
    global syscall_test

syscall_test:
    ; System call: write(int fd, const char *buf, size_t count)
    ; eax = 4 (sys_write)
    ; ebx = file descriptor (1 for stdout)
    ; ecx = pointer to message
    ; edx = length of message

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    mov ecx, message
    mov edx, message_length
    int 0x80                       ; make system call using int 0x80

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    mov ecx, message
    mov edx, message_length
    int 0x80                       ; make system call using int 0x80

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    mov ecx, message
    mov edx, message_length
    int 0x80                       ; make system call using int 0x80

    ; jmp syscall_test  ; just loop for now


    ; System call: exit(int status)
    ; eax = 1 (sys_exit)
    ; ebx = exit status (0 for success)

    mov eax, 1                     ; sys_exit system call number
    mov ebx, 0                     ; exit status 0 (success)
    int 0x80                       ; make system call using int 0x80

