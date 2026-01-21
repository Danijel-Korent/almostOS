
global switch_process
global schedule_in_irq_context

extern get_current_process
extern move_to_next_process
extern print_process_ctx

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



; Implementation of void switch_process(struct process_ctx *current_proc, struct process_ctx *next_proc);
switch_process:

    ; We don't pop this two pushes. Instead, when storing SP, we just move the current value of SP to a position before these pushes
    ; So the stored SP for current_proc will be correct, and afterwards poping doesn't matter anymore as we load SP from next_proc anyway
    pushfd      ; Push the flags register (EFLAGS)
    push eax    ; we will use eax to hold pointer to struct process_ctx

    ; Positions on stack relative to ESP after "pushfd" and "push EAX"
    NEXT_PROC_POS_STACK     equ 16  ; Orig stack pos + ret addr + eflags + eax
    CURRENT_PROC_POS_STACK  equ 12  ; Orig stack pos + ret addr + eflags + eax
    RET_ADDR_STACK      equ 8
    EFLAGS_POS_STACK    equ 4
    EAX_POS_STACK       equ 0

    ;
    ; Save current register values into current_proc
    ;
    mov eax, [esp + CURRENT_PROC_POS_STACK] ; skip over pushed eax, flags and return address, to get the current_proc

    mov [eax + BX_POS], ebx
    mov [eax + CX_POS], ecx
    mov [eax + DX_POS], edx
    mov [eax + SI_POS], esi
    mov [eax + DI_POS], edi
    mov [eax + BP_POS], ebp

    ; save current_proc->reg_esp
    ; Save stack pointer without eflags, and eax pushed on stack in this function + without return address pushed by the caller
    ; When the scheduler later switches back to this saved proccess, it will start executing right after 'call switch_process',
    ; as if it just returned from the 'call' instruction. Therefore, the stack must be also at the value expected after 'call' returns,
    ; that is, without the 'return value' pushed by 'call' instruction, which is poped by 'ret' instruction
    mov edx, esp
    add edx, RET_ADDR_STACK + 4       ; Rewind stack address up to the position without return address, at the value it should be after 'call' returns
    mov [eax + SP_POS], edx

    ; save current_proc->reg_ip
    mov edx, [esp + RET_ADDR_STACK]   ; IP = return address stored on stack
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
    mov eax, [esp + NEXT_PROC_POS_STACK] ; skip over pushed eax, flags, return address and current_proc, to get the next_proc

    mov ebp, [eax + BP_POS]
    mov edi, [eax + DI_POS]
    mov esi, [eax + SI_POS]
    mov ecx, [eax + CX_POS]
    mov edx, [eax + DX_POS]
    mov ebx, [eax + BX_POS]

    ; restore current_proc->reg_eflags
    push dword [eax + FLAGS_POS]
    popfd

    ; restore current_proc->reg_esp
    mov esp, [eax + SP_POS]

    ; restore current_proc->reg_ip
    push dword [eax + IP_POS] ; Push IP/PC address so we can use "ret" to jump to other process code

    ; restore next_proc->reg_eax
    push edx    ; TODO: I see no point in pushing/poping edx here. Copy/paste accident? - Re-check when fully awake
    mov eax, [eax + AX_POS]
    pop edx

; for debugging
.infinite_loop:
    ; jmp .infinite_loop

    ret


; implementation of void switch_process_in_irq(struct process_ctx *current_proc, struct process_ctx *next_proc)
; WARNING: This function needs guarantee that on stack there is only IRQ context stack plus two 32-bit process pointers
; WARNING: This is hard-coded for interrupt without privilege mode chage. If privilege change, then
;          SS and ESP are also pushed on the stack (before EFLAGS)
switch_process_in_irq:

    push eax

    ; Positions on stack relative to the ESP after "push eax"
    ; TODO: I don't like this one bit. It is very inflexible and prone to change
    ;       It would be better to values of everything we need as function arguments. The problem is, it would
    ;       still look very similar to this. But at least stack structure of two functions above would not be hardcoded,
    ;       instead using just standard cdecl arguments by the caller
    EFLAGS_POS_STACK_IRQ        equ 28
    CS_POS_STACK_IRQ            equ 24
    EIP_POS_STACK_IRQ           equ 20
    ;RET_ADDR2_POS_STACK_IRQ    equ 16
    NEXT_PROC_POS_STACK_IRQ     equ 12  ; Orig stack pos + ret addr + eax
    CURRENT_PROC_POS_STACK_IRQ  equ  8  ; Orig stack pos + ret addr + eax
    ;RET_ADDR_POS_STACK_IRQ     equ  4
    EAX_POS_STACK_IRQ           equ  0

    ;
    ; Save current register values into current_proc
    ;

    ; set eax to current_proc
    mov eax, [esp + CURRENT_PROC_POS_STACK_IRQ]

    mov [eax + BX_POS], ebx
    mov [eax + CX_POS], ecx
    mov [eax + DX_POS], edx
    mov [eax + SI_POS], esi
    mov [eax + DI_POS], edi
    mov [eax + BP_POS], ebp

    ; Save stack pointer without eflags and eax pushed on stack in this function
    mov edx, esp
    add edx, EFLAGS_POS_STACK_IRQ + 4   ; Rewind stack address up to the position before eip/cs/eflags were being pushed by "int"
    mov [eax + SP_POS], edx

    ; save current_proc->reg_ip
    mov edx, [esp + EIP_POS_STACK_IRQ]   ; IP = return address stored on stack
    mov [eax + IP_POS], edx

    ; save current_proc->reg_eflags
    mov edx, [esp + EFLAGS_POS_STACK_IRQ]    ; read eflags pushed on stack earlier
    mov [eax + FLAGS_POS], edx

    ; save current_proc->reg_eax
    mov edx, [esp + EAX_POS_STACK_IRQ]  ; eax was pushed to stack, we load it now into edx
    mov [eax + AX_POS], edx             ; save value of eax into structure current_proc

    ; DO NOT POP EAX! This would change all the values of *_POS_STACK_IRQ for the "restore next_proc" part

    push eax
    ;call print_process_ctx
    pop  eax

    ;
    ; Restore register values from next_proc
    ;

    ; set eax to next_proc
    mov eax, [esp + NEXT_PROC_POS_STACK_IRQ]

    push eax
    ;call print_process_ctx
    pop  eax

    mov ebp, [eax + BP_POS]
    mov edi, [eax + DI_POS]
    mov esi, [eax + SI_POS]
    mov ecx, [eax + CX_POS]
    mov edx, [eax + DX_POS]
    mov ebx, [eax + BX_POS]

    ; restore esp
    mov esp, [eax + SP_POS]

    ; Prepare stack frame for the "iret" instruction
    ; When interrupt is executed, CPU pushes eflags, CS and PC/IP registers to stack. "IRET" then pops them while restoring these values into CPU registers
    push dword [eax + FLAGS_POS]
    push dword 0x8  ; TODO: CS is hardcoded !!
    push dword [eax + IP_POS]

    ; restore next_proc->reg_eax
    push edx    ; TODO: I see no point in pushing/poping edx here. Copy/paste accident? - Re-check when fully awake
    mov eax, [eax + AX_POS]
    pop edx

.infinite_loop:
    ; jmp .infinite_loop    ; for debugging

    iret


; implementation of void schedule_in_irq_context(void)
; WARNING: This functions needs guarantee that the stack/regs are in identical conditions as they were at the start of 
;          interrupt handler (plus return address put by "call")
; TODO: Maybe I should just write a C function with args already pushed to stack by CPU when IRQ happens, and then use
;       their addresses to pass to "switch_process" version for IRQ context
schedule_in_irq_context:

    ; reserve space for two pointers to "struct process_ctx"
    push dword 0    ; struct process_ctx *next_proc
    push dword 0    ; struct process_ctx *current_proc

    ; Save state before calling C functions
    pushad      ; Push all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) - WARNING: Not available on x86-64 !
    pushfd      ; Push the flags register (EFLAGS)

    NEXT_PROC_POS_STACK_IRQ1       equ (8*4+4)+4
    CURRENT_PROC_POS_STACK_IRQ1    equ (8*4+4)+0     ; pushad (8x 4bye) + pushfd (4 byte)

    ; TODO: call get_current_process()
    ;       return value is saved in EAX -> https://en.wikipedia.org/wiki/X86_calling_conventions
    ; TODO: save it into reserved stack space above pushad and pushfd
    call get_current_process
    mov [esp + CURRENT_PROC_POS_STACK_IRQ1], eax

    ; TODO: call move_to_next_process()
    ; TODO: save it into reserved stack space above pushad and pushfd
    call move_to_next_process ; TODO: Enable this when saving to struct process_ctx *current_proc is implemented
    mov [esp + NEXT_PROC_POS_STACK_IRQ1], eax

    ; Restore state after C code
    popfd       ; Restore the flags register
    popad       ; Restore all general-purpose registers

    ; TODO: everything after this is never called becase of "iret" being executed inside switch_process_in_irq
    ;       Maybe just 'jmp' instead of 'call' so it is obvious that we will not return here?
    call switch_process_in_irq

    ; Remove 2x dword allocated to the stack at the start of the function
    ; TODO: rewind SP to emulate pop-ing of those two dword pushes at the start
    ; pop eax
    ; pop eax

    ; ret ; TODO: Maybe I can just call "iret" here 
        ;      (but for that I need to remove "return address" from the stack to make it as original, or do jump instead of call)
