/*******************************************************************************
 *                         simple stack implementation                         *
 *******************************************************************************/

#include "stack.h"


// Add size parameter and check if buffer is big enough for it??
bool stack_init(stack_handle_t* const handle, u8* const buffer, u32 buffer_size)
{
    if (handle == NULL || buffer == NULL || buffer_size == 0) return false; // Log error

    handle->buffer_start = buffer;
    handle->buffer_size = buffer_size;

    handle->stack_pointer = buffer;

    return true;
}

bool stack_push(stack_handle_t* const handle, u8 value)
{
    if (handle == NULL) return false; // Log error

    if (handle->stack_pointer > (handle->buffer_start + handle->buffer_size -1))
    {
        return false; // The stack is full
    }

    *handle->stack_pointer = value;
     handle->stack_pointer++;

     return true;
}

bool stack_pop(stack_handle_t* const handle, u8* const value)
{
    if (handle == NULL || value == NULL) return false; // Log error

    if (handle->stack_pointer == handle->buffer_start)
    {
        return false; // The stack is empty
    }

    handle->stack_pointer--;
    *value = *handle->stack_pointer;

    // Clean the garbage that is left behind. Not necessary, but to make potential bugs more predictable/reproducible
    *handle->stack_pointer = 0;

    return true;
}


int run_unittests_stack(void)
{
    int failed_test_counter = 0;

    stack_handle_t handle;
    static u8 buffer[3];

    // TODO: test null pointer arguments

    if (stack_init(&handle, buffer, sizeof(buffer)))
    {
        u8 returned_val = 0;

        if (stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is empty and should not return value
        }

        if ( ! stack_push(&handle, 1))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if ( ! stack_push(&handle, 2))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if ( ! stack_push(&handle, 3))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if (stack_push(&handle, 4))
        {
            failed_test_counter++; // Stack is FULL and should fail to push the value
        }


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 3) failed_test_counter++; // Wrong value


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 2) failed_test_counter++; // Wrong value


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 1) failed_test_counter++; // Wrong value

        if (stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is empty and should not return value
        }
    }

    return failed_test_counter;
}
