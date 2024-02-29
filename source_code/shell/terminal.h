#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include "kernel_stddef.h"


#define TERMINAL_MAX_X (80)
#define TERMINAL_MAX_Y (25)

#define ENABLE_INPUT_LINE  (true)
#define DISABLE_INPUT_LINE (false)

typedef struct terminal_contex_tag
{
    char buffer[TERMINAL_MAX_Y][TERMINAL_MAX_X]; // Circular buffer
    char input_line[TERMINAL_MAX_X - 2]; // Holding user input
    int  input_cursor_position;
    int  window_position_y;     // Physical position on the display (in text lines)
    int  window_size_y;         // Size on the display (in text lines)
    int  current_first_line;    // Position of the first line to be displayed
    int  current_end_line;      // Position of the end of the buffer
    bool is_input_enabled;
} terminal_contex_t; // TODO: typo --> context

void terminal_init          (terminal_contex_t *terminal_context, int window_position_y, int window_size_y, bool is_input_enabled);
void terminal_printline     (terminal_contex_t *terminal_context, const unsigned char* const string);
void terminal_render_to_VGA (terminal_contex_t *terminal_context);
void terminal_on_keypress   (terminal_contex_t *terminal_context, unsigned char key);

#endif // _TERMINAL_H_