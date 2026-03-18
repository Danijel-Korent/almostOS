#ifndef _SHELL_H_
#define _SHELL_H_

#include "kernel_stddef.h"

void shell_init(void);
void shell_on_input(char key);
void shell_execute_command(u8 * input);

const char* get_current_dir(void);
void set_current_dir(char *path);

#endif // _SHELL_H_
