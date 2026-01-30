#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

// TODO: this exposes x86 specifics, but will deal with it later
#include "arch_scheduler.h" // For struct process_ctx; TODO: make struct process_ctx opaque

struct process_internals
{
    u32 stack_mem_start;
    u32 stack_pointer;
    u32 stack_base;
    u32 ip;     // PC
};

void scheduler_init(void);

void schedule(void);
void schedule_in_irq_context(void);

struct process_ctx * get_current_process(void);
struct process_ctx * move_to_next_process(void);

void create_process(const char* name, void (*func_ptr)(void));
void kill_process(int process_index);
void kill_current_process(void);

const char* get_process_name(int process_index);
struct process_internals get_process_internals(int process_index);
int get_max_process_count(void);

//// Test code ////

void test_thread_2_handler(void);
void test_thread_3_handler(void);
void test_thread_4_handler(void);

#endif // _SCHEDULER_H_
