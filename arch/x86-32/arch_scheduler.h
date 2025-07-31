#ifndef _ARCH_SCHEDULER_H_
#define _ARCH_SCHEDULER_H_

#include "kernel_stddef.h"

struct process_ctx
{
    u32 reg_esp;
    const char* name;
};

void switch_process(struct process_ctx *current_proc, struct process_ctx *next_proc);

#endif
