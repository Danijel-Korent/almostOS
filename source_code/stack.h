#ifndef _STACK_H_
#define _STACK_H_

#include "kernel_stddef.h"


typedef struct stack_handle_tag
{
    //int element_size; // Not used because implementation is harcoded to 1byte values
    int buffer_size;
    u8* buffer_start;
    u8* stack_pointer;
} stack_handle_t;

bool stack_init(stack_handle_t* const handle, u8* const buffer, u32 buffer_size);
bool stack_push(stack_handle_t* const handle, u8 value);
bool stack_pop (stack_handle_t* const handle, u8* const value);

int run_unittests_stack(void);

#endif // _STACK_H_
