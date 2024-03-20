#include "shell.h"

#include "string.h"
#include "kernel_main.h"
#include "kernel_stdio.h"

#include "cmd_hexdump.h"
#include "cmd_cat.h"
#include "cmd_ls.h"


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

// TODO: Temp code for testing!
void shell_input(u8 * input)
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

    if (input[0] == 'c')
    {
        execute__cat(argc, argv);
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
        kernel_println("    dump x   - Dump content of the memory at address x");
        kernel_println("    help     - Prints this message");
        kernel_println("");
    }
    else if (input[0] == 'l')
    {
        execute__ls(argc, argv);
    }
    else
    {
        kernel_println("");
        kernel_println("Unknown command!");
        kernel_println("");
    }

    free(argv);
}
