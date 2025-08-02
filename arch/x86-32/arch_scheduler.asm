
global switch_process

; PROTO: void switch_process(struct process_ctx current_proc, struct process_ctx next_proc);
switch_process:
    pushad      ; Push all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) - WARNING: Not available on x86-64 !
    pushfd      ; Push the flags register (EFLAGS)

    ; save sp to current_proc
    mov eax, [esp+4+36] ; load the pointer argument *current_proc into eax. 9 valus of 4b are already pushed to stack by pushad and pushfd
    mov [eax+0], esp    ; process_ctx.reg_esp is first member so offset is 0

    ; load sp from next_proc
    mov eax, [esp+8+36]
    mov esp, [eax+0]    ; Switches the stack from "current_proc" to "next_proc"

    popfd       ; Restore the flags register
    popad       ; Restore all general-purpose registers
    ret         ; ret is the one that actually switch to execting "next_proc" by jumping to "return address" on the stack of "next_proc"
