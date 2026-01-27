#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "arch_scheduler.h" // For struct process_ctx

void scheduler_init(void);

void schedule(void);
void schedule_in_irq_context(void);

struct process_ctx * get_current_process(void);
struct process_ctx * move_to_next_process(void);

void kill_current_process(void);

//void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void));


#endif // _SCHEDULER_H_
