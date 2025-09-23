#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "arch_scheduler.h" // For struct process_ctx

void scheduler_init(void);
void schedule(void);

//void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void));


#endif // _SCHEDULER_H_
