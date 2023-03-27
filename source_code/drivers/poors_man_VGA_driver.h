#ifndef _POORS_MAN_VGA_DRIVER_H_
#define _POORS_MAN_VGA_DRIVER_H_

#include "kernel_stddef.h"


void set_cursor_position(u16 x, u16 y);
void print_char_to_VGA_display(u8 x, u8 y, u8 ch);
void print_char_to_VGA_display_buffer(u16 position, u8 ch);
void print_string_to_VGA_display_buffer(u16 position, unsigned char* string, int string_size);


#endif // _POORS_MAN_VGA_DRIVER_H_
