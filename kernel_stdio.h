#ifndef _KERNEL_STDIO_H_
#define _KERNEL_STDIO_H_

#include "kernel_stddef.h"

void kernel_stdio_init(void);

void LOG(const unsigned char* const msg);
void kernel_putchar(const char new_char);
void kernel_println(const unsigned char* const msg);
void kernel_printf(const char* format, ...);

#endif // _KERNEL_STDIO_H_
