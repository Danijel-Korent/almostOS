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
// TODO: Depricated, need to remove this, along with the "terminal" code
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

static void kernel_print(const unsigned char* const message)
{
    terminal_printline(&shell_terminal, message);

    for (int i = 0; true; i++)
    {
        if (message[i] == 0) break;

        tty_write(message[i]);
    }
}

// Prints to display in Shell "window"
void kernel_println(const unsigned char* const message)
{
    kernel_print(message);
    tty_write('\n');
}


static void print_decimal(int val, char* buffer, int* buf_i, int maxlen, int width, int zero_pad)
{
    char numbuf[16];
    int num_i = 0;
    int is_negative = 0;
    if (val < 0) {
        is_negative = 1;
        val = -val;
    }
    do {
        numbuf[num_i++] = '0' + (val % 10);
        val /= 10;
    } while (val && num_i < (int)sizeof(numbuf));

    // Apply zero-padding if specified (but not for negative numbers)
    if (zero_pad && width > num_i && !is_negative) {
        int padding = width - num_i;
        while (padding-- > 0 && *buf_i < maxlen - 1) {
            buffer[(*buf_i)++] = '0';
        }
    }

    if (is_negative && *buf_i < maxlen - 1)
        buffer[(*buf_i)++] = '-';
    while (num_i-- > 0 && *buf_i < maxlen - 1)
        buffer[(*buf_i)++] = numbuf[num_i];
}

static void print_hex(unsigned int val, char* buffer, int* buf_i, int maxlen, int width, int zero_pad, int uppercase)
{
    char numbuf[16];
    int num_i = 0;
    do {
        unsigned int digit = val % 16;
        if (uppercase) {
            numbuf[num_i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        } else {
            numbuf[num_i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        }
        val /= 16;
    } while (val && num_i < (int)sizeof(numbuf));

    // Apply zero-padding if specified
    if (zero_pad && width > num_i) {
        int padding = width - num_i;
        while (padding-- > 0 && *buf_i < maxlen - 1) {
            buffer[(*buf_i)++] = '0';
        }
    }

    while (num_i-- > 0 && *buf_i < maxlen - 1)
        buffer[(*buf_i)++] = numbuf[num_i];
}

void kernel_printf(const char* format, ...)
{
    char buffer[256];
    int buf_i = 0;

    // Get pointer to first argument after format
    unsigned int* arg_ptr = (unsigned int*)(&format + 1);
    int arg_index = 0;

    for (int i = 0; format[i] != '\0' && buf_i < (int)(sizeof(buffer) - 1); i++)
    {
        if (format[i] == '%')
        {
            i++;

            // Parse flags and width
            int width = 0;
            int zero_pad = 0;

            // Check for zero-padding flag
            if (format[i] == '0') {
                zero_pad = 1;
                i++;
            }

            // Parse width (simple integer parsing)
            while (format[i] >= '0' && format[i] <= '9') {
                width = width * 10 + (format[i] - '0');
                i++;
            }

            if (format[i] == 's')
            {
                const char* s = (const char*)arg_ptr[arg_index++];
                if (s)
                {
                    for (; *s && buf_i < (int)(sizeof(buffer) - 1); s++)
                        buffer[buf_i++] = *s;
                }
            }
            else if (format[i] == 'd')
            {
                int val = (int)arg_ptr[arg_index++];
                print_decimal(val, buffer, &buf_i, sizeof(buffer), width, zero_pad);
            }
            else if (format[i] == 'x')
            {
                unsigned int val = (unsigned int)arg_ptr[arg_index++];
                print_hex(val, buffer, &buf_i, sizeof(buffer), width, zero_pad, 0);
            }
            else if (format[i] == 'X')
            {
                unsigned int val = (unsigned int)arg_ptr[arg_index++];
                print_hex(val, buffer, &buf_i, sizeof(buffer), width, zero_pad, 1);
            }
            else if (format[i] == 'c')
            {
                char c = (char)arg_ptr[arg_index++];
                buffer[buf_i++] = c;
            }
            else
            {
                // Unknown format, just print it
                buffer[buf_i++] = '%';
                if (format[i] != '\0' && buf_i < (int)(sizeof(buffer) - 1))
                    buffer[buf_i++] = format[i];
            }
        }
        else
        {
            buffer[buf_i++] = format[i];
        }
    }
    buffer[buf_i] = '\0';

    kernel_print((const unsigned char*)buffer);

    //return buf_i;
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
    tty_write(key);

    // TODO: I have to rip out this "terminal" monstrosity that is lodged between the kernel and the shell
    //       And move it into x86-32 (although it is arch agnostic, it will never be used on RISC-V)
    terminal_on_keypress(&shell_terminal, key);

#if 0
    char string[] = "Key is pressed:  ";
    string[sizeof(string) -2] = key;
    terminal_printline(&shell_terminal, string);
    //LOG(string);
#endif
}
