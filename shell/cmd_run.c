#include "cmd_run.h"


#include "kernel_stdio.h"
#include "scheduler.h"
#include "util.h"
#include "string.h"
#include "system_headers/string.h" // TODO: I need to do something about util/string.h and system_headers/string.h
#include "shell.h"
#include "filesystem_router.h"

#include "linux_x86_userspace_test.bin.h"


void syscall_test(void);

struct test_list_field
{
    const char name[32];
    void (*process_ptr)(void);
};

static struct test_list_field test_list[] = {
    {"Syscall Test", syscall_test},
    {"linux_x86_userspace_test_bin", (void (*)(void)) linux_x86_userspace_test_bin},
    {"test_thread_2", test_thread_2_handler},
    {"test_thread_3", test_thread_3_handler},
    {"test_thread_4", test_thread_4_handler},
};


static void command_run_test(const char* test_num_str)
{
    if (test_num_str == NULL)
    {
        kernel_println("run: filename is NULL!");
    }

    int test_num = atoi(test_num_str);

    kernel_printf("\nExecuted command run with arg: %d \n", test_num);

    if (test_num < (sizeof test_list / sizeof test_list[0]))
    {
        create_process(test_list[test_num].name, test_list[test_num].process_ptr);
    }
}

void execute__run(int argc, char* argv[])
{
    if (argc > 1)
    {
        const char *subcommand = "test";

        if(strncmp(argv[1], subcommand, 30) == 0)
        {
            if (argc > 2)
            {
                command_run_test(argv[2]);
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
        else
        {
            const char* filename = argv[1];

            // TODO: This code is C/P-ed from cmd_hexdunp (where it is copied from cmd_cat)
            char abs_path[100] = "";

            {
                const char *arg = filename;

                if (arg[0] == '/')
                {
                    // Already absolute path, no need to use current dir
                    mem_copy(abs_path, sizeof abs_path, arg, strlen_unsafe(arg)+1);
                }
                else
                {
                    // Relative path, use current dir to get absolute path
                    const char *current_dir = get_current_dir();
                    append_string(abs_path, sizeof abs_path, current_dir, strlen_unsafe(current_dir)+1);
                    append_string(abs_path, sizeof abs_path, arg, strlen_unsafe(arg)+1);
                }
            }

            kernel_printf("\n[run] file path: %s \n\n", abs_path);

            // TODO: Hardcoded size
            static char file_buffer[256] = {0};

            int ret_len = read_file(abs_path, file_buffer, sizeof file_buffer);

            if (ret_len > 0)
            {
                // TODO: Now the .text is in "static char file_buffer[256]". Running "run file" again will overwrite it
                //       We need to dynamicaly allocate space for the program and copy it there. Deallocate it when the process is killed
                create_process(abs_path, (void (*)(void)) file_buffer);
            }
            else
            {
                kernel_printf("\n[run] Unable to read file! ret = %d \n\n", ret_len);
            }
        }
    }
    else
    {
        kernel_println("\nrun: Missing argument! \n\nUsage: 'run test n' or 'run filename' \n");
    }
}
