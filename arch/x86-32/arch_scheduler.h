#ifndef _ARCH_SCHEDULER_H_
#define _ARCH_SCHEDULER_H_

#include "kernel_stddef.h"

struct process_ctx
{
    u32 reg_esp;
    u32 reg_eax;
    u32 reg_ebx;
    u32 reg_ecx;
    u32 reg_edx;
    u32 reg_esi;
    u32 reg_edi;
    u32 reg_ebp;
    u32 reg_ip;     // PC
    u32 reg_eflags;
    const char* name;
};

void switch_process(struct process_ctx *current_proc, struct process_ctx *next_proc);

#endif
