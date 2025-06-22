/*******************************************************************************
 *                         Poor man's keyboard driver                          *
 *******************************************************************************/

#include "poors_man_keyboard_driver.h"
#include "instruction_wrappers.h"

#include "util.h"
#include "kernel_stdio.h"


static u8 convert_scancode_to_ASCII(u8 scan_code)
{
    // Quick and dirty solution for reading at least letters and numbers
    // Proper solution will need a lot more code to check/support scan code sets,
    // support upper/lower cases, control/alt/shift combinations...
    static u8 scancode_to_ASCII_table[128] =
    {
        // 0    1     2     3     4     5     6     7     8     9     10    11    12    13    14    15
        0x3F, 0x3F,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',  //  0
         'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n', 0x3F,  'a',  's',  // 16
         'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', 0x27, 0x3F, 0x3F, '\\',  'z',  'x',  'c',  'v',  // 32
         'b',  'n',  'm',  ',',  '.',  '/', 0x3F, 0x3F, 0x3F,  ' ', 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  // 48
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  '7',  '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',  // 64
         '2',  '3',  '0', 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  // 80
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  // 96
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  // 112
    };

#if 1
    // For debugging
    if (scancode_to_ASCII_table[scan_code & ~0x80] == 0x3F)
    {
        char code_str[15] = "AbcdAbcd:?";

        long_to_hex(scan_code & ~0x80, code_str, 8, 16);

        kernel_println(code_str);
    }
#endif

    return scancode_to_ASCII_table[scan_code & ~0x80];
}


void keyboard_driver_poll(void)
{
    static u8 previous_scan_code = 0;

    // TODO: Before reading output, check if any data is actually pending
    u8 scan_code = read_byte_from_IO_port(0x60);

    if (scan_code == 0xFA) return;

    if ((previous_scan_code != scan_code) && ((scan_code & 0x80) == 0x80))
    {
        u8 ASCII_code = convert_scancode_to_ASCII(scan_code);
        event_on_keypress(ASCII_code);
    }

    previous_scan_code = scan_code;
}
