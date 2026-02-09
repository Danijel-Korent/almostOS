/**
 * @file terminal.c
 *
 * @brief New terminal code that suck little bit less than the previous one. Has hardcoded input line, doesn't understand
 *        CR and or LF chars, and will cut out lines longer than 80 chars (and doesn't understand any escape chars)
 *
 * @todo This terminal is based on a circular buffer of fixed-sized lines. That is easier to render on VGA but it's a bad
 *       inflexible design, and should be replaced with just a regular char circular buffer, which is then being rendered
 *       independently according to parameters of the terminal window
 */

#include "terminal.h"
#include "tty.h"

#include "shell.h"
#include "string.h"
#include "kernel_main.h"
#include "kernel_stdio.h"
#include "poors_man_VGA_driver.h"

// https://en.wikipedia.org/wiki/ANSI_escape_code
// https://en.wikipedia.org/wiki/VGA_text_mode

// TODO: This needs to be refactored as it is ugly and barely readable
// TODO/BUG: You are not suposed to be able to delete the shell prompt :O

// Local function definitions
static void terminal_clear_input_line(terminal_contex_t *terminal_context);
static void terminal_clear_VGA(terminal_contex_t *terminal_context);
static void terminal_render_to_VGA(terminal_contex_t *terminal_context);

//#define DISABLE_TERMINAL

// TODO: Add argument for no-input mode
void terminal_init(terminal_contex_t *terminal_context, int window_position_y, int window_size_y, bool is_input_enabled)
{
#ifndef DISABLE_TERMINAL
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

    terminal_context->window_size_y = window_size_y;
    terminal_context->window_position_y = window_position_y;
    terminal_context->current_first_line = 0;
    terminal_context->current_end_line = 0; // TODO: initialize with one (empty) line to avoid special case of empty
                                            //       circular line buffer??

    terminal_clear_VGA(terminal_context);
    //terminal_clear_input_line(terminal_context);
    terminal_render_to_VGA(terminal_context);
#endif
}

static int terminal_get_current_line_count(terminal_contex_t *terminal_context)
{
    int first_line = terminal_context->current_first_line;
    int end_line   = terminal_context->current_end_line;

    if (end_line >= first_line) return end_line - first_line;

    // TODO
    return TERMINAL_MAX_Y - first_line + end_line;
}

static int terminal_get_next_line_buffer(terminal_contex_t *terminal_context, int current_line)
{
    int next_line = current_line + 1;

    // Wrap-around
    if (next_line >= TERMINAL_MAX_Y) next_line = 0;

    return next_line;
}


static char* terminal_get_next_free_line_buffer(terminal_contex_t *terminal_context)
{
    int free_line_number = terminal_context->current_end_line;

    if (free_line_number >= TERMINAL_MAX_Y)
    {
        printf("ERROR: terminal_get_next_free_line_buffer - current_end_line out of buffer bounds");
        return NULL;
    }

    // TODO: what the hell is this??
    // TODO: rename terminal_context to terminal_ctx
    char* free_line = terminal_context->buffer[terminal_context->current_end_line];

    // Forward the end line
    terminal_context->current_end_line = terminal_get_next_line_buffer(terminal_context, terminal_context->current_end_line);

    int line_count = terminal_get_current_line_count(terminal_context);

    // If terminal holds more then max number of lines, forward also the first line
    if (line_count > terminal_context->window_size_y)
    {
        terminal_context->current_first_line = terminal_get_next_line_buffer(terminal_context, terminal_context->current_first_line);
    }

    for(int i = 0; i < TERMINAL_MAX_X; i++)
    {
        free_line[i] = 0;
    }

    return free_line;
}


static void terminal_clear_VGA(terminal_contex_t *terminal_context)
{
    int y_start = terminal_context->window_position_y;
    int y_size =  terminal_context->window_size_y;

    for (int y = y_start; y < y_start + y_size; y++)
    {
        for (int x = 0; x < TERMINAL_MAX_X; x++)
        {
            print_char_to_VGA_display(x, y, ' ', DEFAULT_BACKGROUND_COLOR, DEFAULT_FOREGROUND_COLOR);
        }
    }
}

static void terminal_render_to_VGA(terminal_contex_t *terminal_context)
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

            // TODO: I have no idead why I put 'x' arg first, but I don't like it (any more)
            print_char_to_VGA_display(x, y, character, DEFAULT_BACKGROUND_COLOR, DEFAULT_FOREGROUND_COLOR);
        }

        current_line = terminal_get_next_line_buffer(terminal_context, current_line);
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

            print_char_to_VGA_display(x, y, character, DEFAULT_BACKGROUND_COLOR, DEFAULT_FOREGROUND_COLOR);
        }
    }
}


void terminal_print(terminal_contex_t *terminal_context, const unsigned char* string)
{
    while(*string != 0)
    {
        terminal_putchar(terminal_context, *string);
        string++;
    }

    terminal_render_to_VGA(terminal_context);
}

void terminal_printline(terminal_contex_t *terminal_context, const unsigned char* string)
{
    terminal_print(terminal_context, string);
    terminal_putchar(terminal_context, '\n');

    terminal_render_to_VGA(terminal_context);
}

// 
void terminal_putchar(terminal_contex_t *terminal_context, const char new_char)
{
#ifndef DISABLE_TERMINAL
    if (terminal_context == NULL)
    {
        printf("ERROR: terminal_init() - terminal_context == NULL");
        return;
    }

    if (new_char == '\n' || new_char == '\r') // Enter
    {
        (void) terminal_get_next_free_line_buffer(terminal_context);
        terminal_context->current_end_x = 0;
    }

    char *line_buf = NULL;

    if (terminal_context->current_end_x >= TERMINAL_MAX_X)
    {
        line_buf = terminal_get_next_free_line_buffer(terminal_context);

        if (line_buf == NULL)
        {
            printf("ERROR: terminal_init() - line == NULL");
            return;
        }
    }
    else
    {
        int last_line = terminal_context->current_end_line - 1;

        if (last_line < 0) last_line = TERMINAL_MAX_Y - 1;

        line_buf = terminal_context->buffer[last_line];
    }

    int pos_x = terminal_context->current_end_x % TERMINAL_MAX_X;

    // TODO: implement CR and LF handling
    line_buf[pos_x] = new_char;

    terminal_context->current_end_x = pos_x + 1;
#endif
}

// TODO: This whole terminal code is trully horrible. Even AI wouldn't write something like this
//       I need to refactor this, or simply remove it
void terminal_on_keypress(terminal_contex_t *terminal_context, unsigned char key)
{
#ifndef DISABLE_TERMINAL
    if (terminal_context->is_input_enabled == DISABLE_INPUT_LINE)
    {
        return;
    }

    if (key >= 32 && key < 127) // Non-special characters
    {
        terminal_context->input_line[terminal_context->input_cursor_position] = key;

        terminal_context->input_cursor_position++;
    }

    if (key == 8 || key == 0x7f) // Backspace
    {
        if (terminal_context->input_cursor_position != 0)
        {
            terminal_context->input_cursor_position--;

            terminal_context->input_line[terminal_context->input_cursor_position] = 0;
        }
    }

    // x86 keyread produces 10 (\n) after pressing enter
    // COM1 produces 13 (\r) after pressing enter
    if (key == '\n' || key == '\r') // Enter
    {
        terminal_printline(terminal_context, terminal_context->input_line);

        // Copy the line
        // Pass the line to callback_process_input()
        // I should really decouple this from terminal_on_keypress()
        shell_input(terminal_context->input_line + 8);

        terminal_clear_input_line(terminal_context);
    }

    terminal_render_to_VGA(terminal_context);
#endif
}

static void terminal_clear_input_line(terminal_contex_t *terminal_context)
{
#ifndef DISABLE_TERMINAL
    // TODO: This should not supposed to be done here, but need to implement more features to do it in shell.c
    // TODO2: For some reason a newline is outputed on COM port after the prompt. Need to see where
    tty_write('\n');
    const char shell_prompt[] = "shell:> ";
    for (int i = 0; i < sizeof(shell_prompt) -1; i++)
    {
        tty_write(shell_prompt[i]);
    }

    terminal_context->input_line[0] = 's';
    terminal_context->input_line[1] = 'h';
    terminal_context->input_line[2] = 'e';
    terminal_context->input_line[3] = 'l'; // I have no clue why I did it this way...
    terminal_context->input_line[4] = 'l';
    terminal_context->input_line[5] = ':';
    terminal_context->input_line[6] = '>';
    terminal_context->input_line[7] = ' ';

    // Clear the rest of the input line
    for (int x = 8; x < sizeof(terminal_context->input_line); x++)
    {
        terminal_context->input_line[x] = 0;
    }

    terminal_context->input_cursor_position = 8;
#endif
}
