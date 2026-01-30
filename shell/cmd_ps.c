#include "cmd_ps.h"

#include "string.h"
#include "kernel_stdio.h"
#include "scheduler.h"


void execute__ps(int argc, char* argv[])
{
    int max_pid_count = get_max_process_count();

    const char* space_for_process_name = "NAME                ";
    kernel_printf("PID    %sPC/IP       SP          BP           Stack start\n", space_for_process_name);
    for(int i = 0; i < max_pid_count; i++)
    {
        const char *name = get_process_name(i);

        if (name != NULL)
        {
            kernel_printf("%02d     %s", i, name);

            int num_of_spaces = strlen_unsafe(space_for_process_name) - strlen_unsafe(name);
            //kernel_printf(" %d ", num_of_spaces);

            for(int i = 0; i < num_of_spaces; i++)
            {
                kernel_putchar(' ');
            }

            struct process_internals proc = get_process_internals(i);
            kernel_printf("0x%08x  0x%08x  0x%08x   0x%08x \n", proc.ip, proc.stack_pointer, proc.stack_base, proc.stack_mem_start);
        }
    }
}
