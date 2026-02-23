#include "shell.h"

#include "string.h"
#include "kernel_main.h"
#include "kernel_stdio.h"
#include "util.h"

#include "cmd_hexdump.h"
#include "cmd_cat.h"
#include "cmd_ls.h"
#include "cmd_run.h"
#include "cmd_kill.h"
#include "cmd_ps.h"
#include "cmd_cd.h"


static char current_dir[100] = "/";

const char* get_current_dir(void)
{
    return current_dir;
}

void set_current_dir(char *path)
{
    // TODO: Check if the directy actually exists
    mem_copy(current_dir, sizeof current_dir, path, strlen_unsafe(path)+1);
    current_dir[sizeof(current_dir)-1] = 0;
}


/**
 * @brief Parse raw input string and returns it in argv/argc format (array of strings/arguments)
 *
 * @param input_string The raw input string taken from command-line
 * @param argc Number of arguments(strings) in the array "argv"
 * @return char** Returns "argv", which is an array of string pointers
 *
 * @note The function just reuses and modifies input string, and all argv pointers
 *       point to a memory space to which "input_string" is pointing
 */
static char** parse_arguments(char* input_string, int* argc)
{
    if (input_string == NULL || argc == NULL)
    {
        LOG("ERROR: parse_arguments() - received NULL argument!");
        return NULL;
    }

    char* trimmed_input = trim_string(input_string);

    if (trimmed_input == NULL) return NULL;

    // Count the number of args, by counting whitespaces between words,
    // we need to know the length of the array before allocating it on heap
    int len = strlen_unsafe(trimmed_input);
    int num_of_args = 0;

    for(int i = 0; i < len; i++)
    {
        // Find whitespace
        if( trimmed_input[i] == ' ' ) num_of_args++;

        // Skip all consecutive whitespaces
        for(; trimmed_input[i] == ' '; i++);
    }

    *argc = num_of_args +1; // +1 because cmd (first word) is also in the list

    // Allocate argv array
    char** argv = malloc(*argc * sizeof(const char*));

    if (argv == NULL)
    {
        LOG("ERROR: parse_arguments() - No free memory!");
    }
    else
    {
        char *string_start = trimmed_input;
        char *string_end =   trimmed_input;

        for(int i = 0; i < *argc; i++)
        {
            // First whitespace symbol marks the end of argument
            for(;(*string_end != ' ') && (*string_end != 0); string_end++);

            // Null-terminate after every word
            // because they will be used in-place
            *string_end = 0;

            argv[i] = string_start;

            // Skip null-terminator and all consecutive whitespaces
            string_end++;
            for(; *string_end == ' '; string_end++);

            string_start = string_end;
        }
    }

    return argv;
}

char input_line[50] = "";
int  input_len = 0;

// TODO: This should event exist on this layer, but it is a workaround for now
void shell_on_input(char key)
{
    kernel_putchar(key);

    if (key == '\n' || key == '\r') // Enter
    {
        shell_execute_command(input_line);
        input_len = 0;
        input_line[0] = 0;
    }
    else if (key == 8 || key == 0x7f) // Del char
    {
        if (input_len > 0)
        {
            input_len--;
            input_line[input_len] = 0;
        }
    }
    else
    {
        // Add char to the input line
        input_line[input_len] = key;
        input_len++;

        input_line[input_len] = 0;
        input_line[sizeof input_line] = 0;
    }
}

// TODO: Temp code for testing!
void shell_execute_command(u8 * input)
{
    if (input == NULL) return;

    int    argc = 0;
    char** argv = NULL;

    argv = parse_arguments(input, &argc);

    if (argv == NULL)
    {
        kernel_println("ERROR: Parsing arguments failed");
        return;
    }

    if (input[0] == 'c') // hehehe
    {
        if (input[1] == 'd') // Oh my!
        {
            execute__cd(argc, argv);
        }
        else
        {
            execute__cat(argc, argv);
        }
    }
    else if (input[0] == 'd')
    {
        // TODO: When we will be able to use COM1 then we will need to also pass pointer interface for read/write
        //       function pointers, to that command read and writes back to correct interface
        execute__dump_data(argc, argv);
    }
    else if (input[0] == 'h')
    {
        kernel_println("");
        kernel_println("Available commands:");
        kernel_println("");
        kernel_println("    ls       - List files");
        kernel_println("    cat file - Output file content");
        kernel_println("    ps       - List processes");
        kernel_println("    kill PID - Kill a process");
        kernel_println("    run TEST - Run a test process");
        kernel_println("    dump x   - Dump content of the memory at address x");
        kernel_println("    help     - Prints this message");
        kernel_println("");
    }
    else if (input[0] == 'k')
    {
        execute__kill(argc, argv);
    }
    else if (input[0] == 'l')
    {
        execute__ls(argc, argv);
    }
    else if (input[0] == 'p')
    {
        execute__ps(argc, argv);
    }
    else if (input[0] == 'r')
    {
        execute__run(argc, argv);
    }
    else if (input[0] == 0)
    {
        // No-op
    }
    else
    {
        kernel_println("");
        kernel_println("Unknown command!");
        kernel_printf("command = %s", input);

        kernel_printf("\n hex =");
        kernel_printf(" %x", input[0]);
        kernel_printf(" %x", input[1]);
        kernel_printf(" %x", input[2]);
        kernel_printf(" %x", input[3]);
        kernel_println("");
    }

    kernel_printf("[shell: %s]$ ", get_current_dir());

    free(argv);
}
