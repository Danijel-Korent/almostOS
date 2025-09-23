
global switch_process


; Positions of members in struct process_ctx
SP_POS equ 0
AX_POS equ 4
BX_POS equ 8
CX_POS equ 12
DX_POS equ 16
SI_POS equ 20
DI_POS equ 24
BP_POS equ 28
IP_POS equ 32
FLAGS_POS equ 36

; Positions on stack
IP_POS_STACK        equ 8
EFLAGS_POS_STACK    equ 4
EAX_POS_STACK       equ 0


switch_process:
    pushfd      ; Push the flags register (EFLAGS)
    push eax    ; we will use eax to hold pointer to struct process_ctx

    ;
    ; Save current register values into current_proc
    ;
    mov eax, [esp+4+4+4] ; skip over pushed eax, flags and return address, to get the current_proc


    mov [eax + BX_POS], ebx
    mov [eax + CX_POS], ecx
    mov [eax + DX_POS], edx
    mov [eax + SI_POS], esi
    mov [eax + DI_POS], edi
    mov [eax + BP_POS], ebp

    ; Save stack pointer without eflags and eax pushed on stack in this function
    mov edx, esp
    add edx, IP_POS_STACK       ; Rewind stack up to the position of return address
    mov [eax + SP_POS], edx

    ; save current_proc->reg_ip
    mov edx, [esp + IP_POS_STACK]   ; IP = return address stored on stack
    mov [eax + IP_POS], edx

    ; save current_proc->reg_eflags
    mov edx, [esp + EFLAGS_POS_STACK]    ; read eflags pushed on stack earlier
    mov [eax + FLAGS_POS], edx

    ; save current_proc->reg_eax
    mov edx, [esp]              ; eax was pushed to stack, we load it now into edx
    mov [eax + AX_POS], edx     ; save value of eax into structure current_proc


    ;
    ; Restore register values from next_proc
    ;
    mov eax, [esp+4+4+4+4] ; skip over pushed eax, flags, return address and current_proc, to get the next_proc

    mov ebp, [eax + BP_POS]
    mov edi, [eax + DI_POS]
    mov esi, [eax + SI_POS]
    mov ecx, [eax + CX_POS]
    mov edx, [eax + DX_POS]
    mov ebx, [eax + BX_POS]

    ; restore current_proc->reg_eflags
    push dword [eax + FLAGS_POS]
    popfd

    mov esp, [eax + SP_POS]

    ; restore current_proc->reg_ip
    ; TODO: In our case the IP will already be on stack

    ; restore next_proc->reg_eax
    ; TODO:
    ;   - Currently we don't need to do this because of cdecl and no preemtive switching
    ;   - How to do it? Push current eax to stack, then read new eax from [esp + AX_POS], and then add esp, 4?


; for debugging
.infinite_loop:
    ; jmp .infinite_loop

    ret



;------------------------------------------------------/ test code for syscall impl. /------------------------------------------------------

global syscall_test
global syscall_entry_point_asm
global INT_80_test


section .data
    msg_int80_test db 0xa, 0ax,'           >>>> Executed INT_80_test sycall ASM !!! <<< ', 0xa   ; String to print with newline (0xa)
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


extern sys_write_test
extern sys_write                ; ssize_t sys_write(int fd, const void *buf, size_t count)
extern syscall_entry_point_C    ; void syscall_entry_point_C(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)

section .data
    msg_sys_test db 0xa, 0ax,'           >>>> CALLED syscall_entry_point ASM !!! <<< ', 0xa   ; String to print with newline (0xa)
    msg_sys_test_len equ $ - msg_sys_test  ; Calculate length of message

section .text


syscall_entry_point_asm:
    pushad      ; Push (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) - WARNING: Not available on x86-64 !
    pushfd      ; Push the flags register (EFLAGS)
    push edx
    push ecx
    push ebx
    push eax

    call syscall_entry_point_C

    pop eax
    pop eax
    pop eax
    pop eax ; I'm layz sometimes. TODO: Just move the stack pointer

    popfd       ; Restore the flags register
    popad       ; Restore all general-purpose registers
    iret

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
    message db 'Hello World', 0xa   ; String to print with newline (0xa)
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

    jmp syscall_test  ; just loop for now


    ; System call: exit(int status)
    ; eax = 1 (sys_exit)
    ; ebx = exit status (0 for success)

    mov eax, 1                     ; sys_exit system call number
    mov ebx, 0                     ; exit status 0 (success)
    int 0x80                       ; make system call using int 0x80
