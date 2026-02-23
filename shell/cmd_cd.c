#include "cmd_cd.h"

#include "kernel_stdio.h"
#include "util.h"
#include "string.h"
#include "shell.h"


void command_cd(const char* arg_path)
{
    char abs_path[100] = "";

    {
        if (arg_path[0] == '/')
        {
            // Absolute path, no need to use current dir
            mem_copy(abs_path, sizeof abs_path, arg_path, strlen_unsafe(arg_path)+1);
        }
        else
        {
            // Relative path, use current dir to get absolute path
            const char *current_dir = get_current_dir();
            append_string(abs_path, sizeof abs_path, current_dir, strlen_unsafe(current_dir)+1);
            append_string(abs_path, sizeof abs_path, arg_path, strlen_unsafe(arg_path)+1);
        }
    }

    // Append '/' if not present
    if (abs_path[strlen_unsafe(abs_path)-1] != '/')
    {
        append_string(abs_path, sizeof abs_path, "/", 2);
    }

    //kernel_printf("\n[cd] file path: %s \n\n", abs_path);

    set_current_dir(abs_path);
}

void execute__cd(int argc, char* argv[])
{
    if (argc > 1)
    {
        command_cd(argv[1]);
    }
    else
    {
        command_cd("/");
    }
}
