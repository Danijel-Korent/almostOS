/**
 * @file terminal.c
 *
 * @brief New terminal code that suck little bit less than previous one. Has hardcoded input line, doesn't understand
 *        CR and or LF chars, and will cut out lines longer than 80 chars
 */

#include "terminal.h"
#include "kernel_stddef.h"

#include "poors_man_VGA_driver.h"

// https://en.wikipedia.org/wiki/ANSI_escape_code
// https://en.wikipedia.org/wiki/VGA_text_mode



// TEMP: TODO: Move this into some header
int printf ( const char * format, ... );

// Local function defitions
static void terminal_clear_input_line(terminal_contex_t *terminal_context);
void input_callback(char * input);


// TODO: Add argument for no-input mode
void terminal_init(terminal_contex_t *terminal_context, int window_position_y, int window_size_y, bool is_input_enabled)
{
    if (terminal_context == NULL)
    {
        printf("ERROR: terminal_init() - terminal_context == NULL");
        return;
    }

    if (window_position_y >= TERMINAL_MAX_Y)
    {
        printf("ERROR: terminal_init() - window_position_y >= TERMINAL_MAX_Y");
        window_position_y = TERMINAL_MAX_Y;
    }

    if (window_size_y >= TERMINAL_MAX_Y)
    {
        printf("ERROR: terminal_init() - window_size_y >= TERMINAL_MAX_Y");
        window_size_y = TERMINAL_MAX_Y;
    }

    terminal_context->is_input_enabled = is_input_enabled;

    // Clear the buffer
    for (int y = 0; y < TERMINAL_MAX_Y; y++)
    {
        for (int x = 0; x < TERMINAL_MAX_X; x++)
        {
            terminal_context->buffer[y][x] = ' ';
        }
    }

    // Clear input line
    terminal_clear_input_line(terminal_context);

    //terminal_context->input_line[0] = 'T';
    //terminal_context->input_line[1] = 'e';
    //terminal_context->input_line[2] = 's';
    //terminal_context->input_line[3] = 'T';
    //terminal_context->input_line[4] = '!';
    //terminal_context->input_cursor_position = 5;

    terminal_context->window_size_y = window_size_y;
    terminal_context->window_position_y = window_position_y;
    terminal_context->current_first_line = 0;
    terminal_context->current_end_line = 0;
}

static int terminal_get_current_line_count(terminal_contex_t *terminal_context)
{
    int first_line = terminal_context->current_first_line;
    int end_line   = terminal_context->current_end_line;

    if (end_line >= first_line) return end_line-first_line;

    // TODO
    return TERMINAL_MAX_Y - first_line + end_line;
}

static int terminal_get_next_line(terminal_contex_t *terminal_context, int current_line)
{
    int next_line = current_line + 1;

    // Wrap-around
    if (next_line >= TERMINAL_MAX_Y) next_line = 0;

    return next_line;
}


static char* terminal_get_next_free_line(terminal_contex_t *terminal_context)
{
    int free_line_number = terminal_context->current_end_line;

    if (free_line_number >= TERMINAL_MAX_Y)
    {
        printf("ERROR: terminal_get_next_free_line - current_end_line out of buffer bounds");
        return NULL;
    }

    char* free_line = terminal_context->buffer[terminal_context->current_end_line];

    // Forward the end line
    terminal_context->current_end_line = terminal_get_next_line(terminal_context, terminal_context->current_end_line);

    int line_count = terminal_get_current_line_count(terminal_context);

    // If terminal holds more then max number of lines, forward also the first line
    if (line_count > terminal_context->window_size_y)
    {
        terminal_context->current_first_line = terminal_get_next_line(terminal_context, terminal_context->current_first_line);
    }

    for(int i = 0; i < TERMINAL_MAX_X; i++)
    {
        free_line[i] = 0;
    }

    return free_line;
}


void terminal_render_to_VGA(terminal_contex_t *terminal_context)
{
    int current_line = terminal_context->current_first_line;

    int line_counter = 0;

    // Draw current content of terminal
    while (current_line != terminal_context->current_end_line)
    {
        for (int x = 0; x < TERMINAL_MAX_X; x++)
        {
            char character = terminal_context->buffer[current_line][x];

            int y = terminal_context->window_position_y + line_counter; // TODO: Move out of loop

            print_char_to_VGA_display(x, y, character);
        }

        current_line = terminal_get_next_line(terminal_context, current_line);
        line_counter++;
    }

    if (terminal_context->is_input_enabled == ENABLE_INPUT_LINE)
    {
        // Draw input line
        for (int x = 0; x < sizeof(terminal_context->input_line); x++)
        {
            char character = terminal_context->input_line[x];

            int y = terminal_context->window_position_y + line_counter; // TODO: Move out of loop

            set_cursor_position(terminal_context->input_cursor_position, y); // TODO: Move out of loop

            print_char_to_VGA_display(x, y, character);
        }
    }
}

void terminal_printline(terminal_contex_t *terminal_context, const unsigned char* const string)
{
    if (terminal_context == NULL)
    {
        printf("ERROR: terminal_init() - terminal_context == NULL");
        return;
    }

    if (string == NULL)
    {
        printf("ERROR: terminal_init() - string == NULL");
        return;
    }

    char *line = terminal_get_next_free_line(terminal_context);

    if (line == NULL)
    {
        printf("ERROR: terminal_init() - line == NULL");
        return;
    }

    for (int i = 0; i < TERMINAL_MAX_X; i++)
    {
        if (string[i] == 0) break;

        line[i] = string[i];
    }

    terminal_render_to_VGA(terminal_context);
}


void terminal_on_keypress(terminal_contex_t *terminal_context, unsigned char key)
{
    if (terminal_context->is_input_enabled == DISABLE_INPUT_LINE)
    {
        return;
    }

    if (key >= 32 && key < 127) // Non-special characters
    {
        terminal_context->input_line[terminal_context->input_cursor_position] = key;

        terminal_context->input_cursor_position++;
    }

    if (key == 8) // Backspace
    {
        if (terminal_context->input_cursor_position != 0)
        {
            terminal_context->input_cursor_position--;

            terminal_context->input_line[terminal_context->input_cursor_position] = 0;
        }
    }

    if (key == 10) // Enter
    {
        terminal_printline(terminal_context, terminal_context->input_line);

        // Copy the line
        // Pass the line to callback_process_input()
        // I should really decouple this from terminal_on_keypress()
        input_callback(terminal_context->input_line + 8);

        terminal_clear_input_line(terminal_context);
    }

    terminal_render_to_VGA(terminal_context);
}

static void terminal_clear_input_line(terminal_contex_t *terminal_context)
{
    // Clear input line //TODO: Move to a function
    terminal_context->input_line[0] = 's';
    terminal_context->input_line[1] = 'h';
    terminal_context->input_line[2] = 'e';
    terminal_context->input_line[3] = 'l';
    terminal_context->input_line[4] = 'l';
    terminal_context->input_line[5] = ':';
    terminal_context->input_line[6] = '>';
    terminal_context->input_line[7] = ' ';
    for (int x = 8; x < sizeof(terminal_context->input_line); x++)
    {
        terminal_context->input_line[x] = 0;
    }

    terminal_context->input_cursor_position = 8;
}


// --------------------------------- TEMP CODE -----------------------------------/

#include "hexdump.h"

void* malloc(int size); // TODO: Move into a header
void free(void* pointer);
void LOG(const unsigned char* const message); // TODO: Move into a header

// TODO: Move this generic helper functions for length and trimming into a common file for helper functions
// TODO: Add postfix "_unsafe" to all unbounded functions
int strlen(const char * input_string)
{
    if (input_string == NULL) return 0;

    int i = 0;

    //for(i = 0; input_string[i] != 0; i++)
    while (input_string[i] != 0)
    {
        i++;

        if (i > 100)
        {
            LOG("ERROR: strlen() smashed protection limit!");
            return 0;
        }
    }

    return i;
}

/**
 * @brief Removes leading and trailing white-space
 *
 * @param input_string String to be trimmed
 * @return char* String that is now trimmed
 *
 * @note The trimming is done 'in-place', meaning returned trimmed string is
 *       in the same memory space in which the original string was
 */
char* trim_string(char* input_string)
{
    if (input_string == NULL)
    {
        LOG("ERROR: trim_string() - received NULL argument!");
        return NULL;
    }

    char *trimmed_string = input_string;

    // Trim leading whitespace
    // Do it by just moving pointer along the orignal input string
    // util the first non whitespace symbol
    for(; *trimmed_string == ' '; trimmed_string++);

    // Trim trailing whitespace
    // Do it by null-terminating from end of string until
    // stumble upon non-whitespace char
    int len = strlen(trimmed_string);

    while(len > 0)
    {
        if(trimmed_string[len -1] != ' ') break;

        // If it is whitespace, nullterminate it
        trimmed_string[len -1] = 0;

        len--;
    }

    return trimmed_string;
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
char** parse_arguments(char* input_string, int* argc)
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
    int len = strlen(trimmed_input);
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


void stdandard_println(const unsigned char* const message); // TOOD: typo "stdandard"


// TODO: Temp code for testing!
void input_callback(char * input)
{
    if (input == NULL) return;

    int    argc = 0;
    char** argv = NULL;

    argv = parse_arguments(input, &argc);

    if (argv == NULL)
    {
        stdandard_println("ERROR: Parsing arguments failed");
        return;
    }

    if (input[0] == 'h')
    {
        stdandard_println("");
        stdandard_println("Available commands:");
        stdandard_println("");
        stdandard_println("    help - prints this message");
        stdandard_println("");
    }
    if (input[0] == 'd')
    {
        execute__dump_data(argc, argv);
    }
    else
    {
        stdandard_println("");
        stdandard_println("Unknown command!");
        stdandard_println("");
    }

    free(argv);
}
