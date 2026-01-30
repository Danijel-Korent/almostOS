#include "cmd_run.h"

#include "kernel_stdio.h"
#include "scheduler.h"
#include "util.h"


void syscall_test(void);

static void (*program_ptr)(void) = syscall_test;

struct test_list_field
{
    const char name[16];
    void (*process_ptr)(void);
};

static struct test_list_field test_list[] = {
    {"Syscall Test", syscall_test},
    {"Syscall Test", syscall_test},
    {"test_thread_2", test_thread_2_handler},
    {"test_thread_3", test_thread_3_handler},
    {"test_thread_4", test_thread_4_handler}
};


void command_run(const char* test_num_str)
{
    if (test_num_str == NULL)
    {
        kernel_println("run: filename is NULL!");
    }

    int test_num = atoi(test_num_str);

    kernel_printf("Executed command run with arg: %d \n", test_num);

    if (test_num < (sizeof test_list / sizeof test_list[0]))
    {
        create_process(test_list[test_num].name, test_list[test_num].process_ptr);
    }
}

void execute__run(int argc, char* argv[])
{
    if (argc > 1)
    {
        command_run(argv[1]);
    }
    else
    {
        kernel_println("\nrun: Missing test number! \n");
        kernel_println("Available tests: \n");

        for (int i = 0; i < (sizeof test_list / sizeof test_list[0]); i++)
        {
            kernel_printf("  %d - %s \n", i, test_list[i].name);
        }
    }
}
