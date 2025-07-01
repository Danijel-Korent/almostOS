#include "kernel_stdio.h"
#include "tty.h"

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
    unsigned char klog_header[] = "--------------------------------/ KERNEL LOG /---------------------------------";
    print_string_to_VGA_display_buffer(19 * 80, klog_header, sizeof(klog_header)-1);

    // New terminal
    terminal_init(&shell_terminal,  0, 17, ENABLE_INPUT_LINE);
    terminal_init(&log_terminal,   20,  5, DISABLE_INPUT_LINE);
}

// TODO: Decouple logging with a queue to avoid circular calls to LOG
// Prints to display in Log "window"
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

// Prints to display in Shell "window"
void kernel_println(const unsigned char* const message)
{
    terminal_printline(&shell_terminal, message);

    for (int i = 0; true; i++)
    {
        if (message[i] == 0) break;

        tty_write(message[i]);
    }

    tty_write('\n');
}

void kernel_putchar(const char new_char)
{
    terminal_putchar(&shell_terminal, new_char);

    tty_write(new_char);
}

/*******************************************************************************
 *                            Integration callbacks                            *
 *******************************************************************************/

// TODO: Move this into callbacks/integration file
void event_on_keypress(u8 key)
{
    // TODO: I have to rip out this "terminal" monstrosity that is lodged between the kernel and the shell
    //       And move it into x86-32 (although it is arch agnostic, it will never be used on RISC-V)
    terminal_on_keypress(&shell_terminal, key);

    tty_write(key);

#if 0
    char string[] = "Key is pressed:  ";
    string[sizeof(string) -2] = key;
    terminal_printline(&shell_terminal, string);
    //LOG(string);
#endif
}
