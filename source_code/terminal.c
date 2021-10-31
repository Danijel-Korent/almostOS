/**
 * @file terminal.c
 *
 * @brief New terminal code that suck little bit less than previous one. Has hardcoded input line, doesn't understand
 *        CR and or LF chars, and will cut out lines longer than 80 chars
 */

#include "terminal.h"
#include "kernel_stddef.h"

// https://en.wikipedia.org/wiki/ANSI_escape_code
// https://en.wikipedia.org/wiki/VGA_text_mode



// TEMP: TODO: Move this into some header
int printf ( const char * format, ... );
void print_char_to_VGA_display(unsigned int x, unsigned int y, unsigned char ch);

// Local function defitions
static void terminal_clear_input_line(terminal_contex_t *terminal_context);
void input_callback(const char * input);


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

            print_char_to_VGA_display(x, y, character);
        }
    }
}

void terminal_printline(terminal_contex_t *terminal_context, char* string)
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

void stdandard_println(const unsigned char* const message); // TOOD: typo "stdandard"

// TODO: Temp code for testing!
void input_callback(const char * input)
{
    if (input == NULL) return;

    if (input[0] == 'h')
    {
        stdandard_println("");
        stdandard_println("Available commands:");
        stdandard_println("");
        stdandard_println("    help - prints this message");
        stdandard_println("");
    }
    else
    {
        stdandard_println("");
        stdandard_println("Unknown command!");
        stdandard_println("");
    }
}
