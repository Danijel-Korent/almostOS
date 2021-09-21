#ifndef _TERMINAL_H_
#define _TERMINAL_H_


#define TERMINAL_MAX_X (80)
#define TERMINAL_MAX_Y (25)

typedef struct terminal_contex_tag
{
    char buffer[TERMINAL_MAX_Y][TERMINAL_MAX_X]; // Circular buffer
    int  window_position_y;     // Physical position on the display (in text lines)
    int  window_size_y;         // Size on the display (in text lines)
    int  current_first_line;    // Position of the first line to be displayed
    int  current_end_line;      // Position of the end of the buffer
} terminal_contex_t;

void terminal_init          (terminal_contex_t *terminal_context, int window_position_y, int window_size_y);
void terminal_printline     (terminal_contex_t *terminal_context, char* string);
void terminal_render_to_VGA (terminal_contex_t *terminal_context);


#endif // _TERMINAL_H_