#include "kernel_stdio.h"

#include "util.h"
#include "string.h"
#include "terminal.h"
#include "instruction_wrappers.h"
#include "poors_man_VGA_driver.h"
#include "poors_man_keyboard_driver.h"


static terminal_contex_t shell_terminal;
static terminal_contex_t log_terminal;

void kernel_stdio_init(void)
{
    // Draw screen outline
    //unsigned char shell_header[] = "-----------------------------------/ Shell /-----------------------------------";
    unsigned char shell_header[] = "-----------------------------------/ SHELL /-----------------------------------";
    print_string_to_VGA_display_buffer(560, shell_header, sizeof(shell_header)-1);

    //unsigned char klog_header[] = "--------------------------------/ Kernel log /---------------------------------";
    unsigned char klog_header[] = "--------------------------------/ KERNEL LOG /---------------------------------";
    print_string_to_VGA_display_buffer(1360, klog_header, sizeof(klog_header)-1);

    // New terminal
    terminal_init(&shell_terminal,  8, 8, ENABLE_INPUT_LINE);
    terminal_init(&log_terminal,   18, 5, DISABLE_INPUT_LINE);
}

void LOG(const unsigned char* const message)
{
    int timestamp = get_timestamp();

    char str_with_timestamp[TERMINAL_MAX_X] = "AbcdAbcd: ";

    long_to_hex(timestamp, str_with_timestamp, 8, 16);

    int message_size = strlen_unsafe_logless(message);

    // TODO: Add terminal_print() (without new line) so we can skip appending
    append_string(str_with_timestamp, sizeof(str_with_timestamp), message, message_size+1); // TODO: +1 because append didn't null-terminate. Fix it

    terminal_printline(&log_terminal, str_with_timestamp);
}

void kernel_println(const unsigned char* const message)
{
    terminal_printline(&shell_terminal, message);
}


/*******************************************************************************
 *                            Integration callbacks                            *
 *******************************************************************************/

// TODO: Move this into callbacks/integration file
void event_on_keypress(u8 key)
{
    terminal_on_keypress(&shell_terminal, key);

#if 0
    char string[] = "Key is pressed:  ";
    string[sizeof(string) -2] = key;
    terminal_printline(&shell_terminal, string);
    //LOG(string);
#endif
}
