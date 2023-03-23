/*******************************************************************************
 *                         Poor man's keyboard driver                          *
 *******************************************************************************/

#include "poors_man_keyboard_driver.h"
#include "instruction_wrappers.h"


static u8 convert_scancode_to_ASCII(u8 scan_code)
{
    // Quick and dirty solution for reading at least letters and numbers
    // Proper solution will need a lot more code to check/support scan code sets,
    // support upper/lower cases, control/alt/shift combinations...
    static u8 scancode_to_ASCII_table[128] =
    {
        0x3F, 0x3F,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', '\b', '\t',
         'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n', 0x3F,  'a',  's',
         'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', 0x27, 0x3F, 0x3F, '\\',  'z',  'x',  'c',  'v',
         'b',  'n',  'm',  ',',  '.',  '/', 0x3F, 0x3F, 0x3F,  ' ', 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
        0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    };

    return scancode_to_ASCII_table[scan_code & ~0x80];
}


void keyboard_driver_poll(void)
{
    static u8 previous_scan_code = 0;

    // TODO: Before reading output, check if any data is actually pending
    u8 scan_code = read_byte_from_IO_port(0x60);

    if ((previous_scan_code != scan_code) && ((scan_code & 0x80) == 0x80))
    {
        u8 ASCII_code = convert_scancode_to_ASCII(scan_code);
        event_on_keypress(ASCII_code);
    }

    previous_scan_code = scan_code;
}
