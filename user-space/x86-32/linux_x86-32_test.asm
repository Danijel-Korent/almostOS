; 32-bit x86 Linux assembly program to print "Hello World"

; Compile:  nasm -f elf32 linux_x86-32_test.asm -o linux_x86-32_test.o
; Link:     ld -m elf_i386 linux_x86-32_test.o -o linux_x86-32_test
;
; A mess with x86 system call instructions in compat mode:
; https://reverseengineering.stackexchange.com/questions/16454/struggling-between-syscall-or-sysenter-windows
; https://stackoverflow.com/questions/77678700/calling-system-api-from-32-bit-processes-under-linux-64-bit/77680771#77680771

; qemu-i386 ./linux_x86-32_test

; MOTE:
;   Commented out .data, and put data between CPU instructions to make exported flat binary smaller. Otherwise objcopy produce .data at 4k address
;   The space is probably added by linker when producing elf file. There is maybe a way to move .text and .data closer without a need for linker script,
;   but I was not in a mood to search how to do it
;
;section .data
;    message db 0xa, 0xa, 'Hello World from the user space!', 0xa   ; String to print with newline (0xa)
;    message_length equ $ - message  ; Calculate length of message


section .text
    global _start                   ; Entry point for linker

_start:
    ; System call: write(int fd, const char *buf, size_t count)
    ; eax = 4 (sys_write)
    ; ebx = file descriptor (1 for stdout)
    ; ecx = pointer to message
    ; edx = length of message
    call get_IP


    ; ---- DATA BETWEEN INSTRUCTIONS ----
    jmp main

    ; I have put data inside .text segment to make exported flat binary smaller
    message db 0xa, 0xa, 0xa, ' >>>> Hello World from the user space! <<<<', 0xa   ; String to print with newline (0xa)
    message_length equ $ - message  ; Calculate length of message

main:
    ; ---- DATA BETWEEN INSTRUCTIONS ----


    lea ecx, [eax + message - _start -5] ; -5 to get the IP address before executing 'call' (the _start), not after

    mov eax, 4                     ; sys_write system call number
    mov ebx, 1                     ; file descriptor 1 (stdout)
    ; mov ecx, message
    mov edx, message_length
    int 0x80                       ; make system call using int 0x80

    ; System call: exit(int status)
    ; eax = 1 (sys_exit)
    ; ebx = exit status (0 for success)

    mov eax, 1                     ; sys_exit system call number
    mov ebx, 0                     ; exit status 0 (success)
    int 0x80                       ; make system call using int 0x80


; Need to read IP to make the binary position independent
get_IP:
    mov eax, [esp]            ; esp contains the return address, the IP of the instruction after 'call'
    ret

    ; Just to serve as visual marker
    message_app_end db '[End of code]'   ; String to print with newline (0xa)
