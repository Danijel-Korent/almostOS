#ifndef _SHELL_H_
#define _SHELL_H_

#include "kernel_stddef.h"

void shell_on_input(char key);
void shell_input(u8 * input);

const char* get_current_dir(void);

#endif // _SHELL_H_
