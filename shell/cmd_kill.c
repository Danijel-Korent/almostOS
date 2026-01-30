#include "cmd_kill.h"

#include "kernel_stdio.h"
#include "scheduler.h"
#include "util.h"


static void command_kill(const char* str_pid)
{
    if (str_pid == NULL)
    {
        kernel_println("kill: argument is NULL!");
    }

    int pid =  atoi(str_pid);

    kill_process(pid);

    kernel_printf("Executed command 'kill' with arg: %d \n", pid);
}

void execute__kill(int argc, char* argv[])
{
    if (argc > 1)
    {
        command_kill(argv[1]);
    }
    else
    {
        kernel_println("cat: Missing arguments!");
    }
}
