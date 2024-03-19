/*******************************************************************************
 *                             VGA Output functions                            *
 *******************************************************************************/

#include "poors_man_VGA_driver.h"
#include "instruction_wrappers.h"

#define VGA_TEXTMODE_WIDTH  (80)
#define VGA_TEXTMODE_HEIGHT (25)


// Registers:
// VGA has more than 300 registers but uses only cca. 18 IO ports. For that reason most registers are "indexed",
// meaning that one port is used for selecting register by setting the index number of register, while another paired
// port is used for writing or reading the data from selected register
// http://www.osdever.net/FreeVGA/vga/portidx.htm

#define CRTC_CONTROLLER_ADDRESS    (0x3D4)
#define CRTC_CONTROLLER_DATA       (0x3D5)

#define CRTC_REG_CURSOR_LOCATION_HIGH    (0xE)
#define CRTC_REG_CURSOR_LOCATION_LOW     (0xF)


void set_cursor_position(u16 x, u16 y)
{
    unsigned int offset = y * VGA_TEXTMODE_WIDTH + x;

    // TODO: Name the addresses and values
    write_byte_to_IO_port(CRTC_CONTROLLER_ADDRESS, CRTC_REG_CURSOR_LOCATION_LOW);
    write_byte_to_IO_port(CRTC_CONTROLLER_DATA, (unsigned char) offset);

    write_byte_to_IO_port(CRTC_CONTROLLER_ADDRESS, CRTC_REG_CURSOR_LOCATION_HIGH);
    write_byte_to_IO_port(CRTC_CONTROLLER_DATA, (unsigned char)(offset >> 8));
}


void print_char_to_VGA_display_buffer(u16 position, u8 ch)
{
    static unsigned char* const VGA_RAM = (unsigned char*) 0x000B8000;

    position = position * 2; // One char takes 16 bits in VGA RAM (8bit char to display + 8bit for color)

    // QTODO: hardcoded color - add arguments for foreground and background color
    VGA_RAM[position]   = ch;

    //VGA_RAM[position+1] = 0x13; // Blue + Cyan
    //VGA_RAM[position+1] = 0x0A; // Black + Light green
    //VGA_RAM[position+1] = 0x02; // Black + Green // This is the correct way because it's how the Hollywood does it
    VGA_RAM[position+1] = 0x07; // Black + White
}

void print_char_to_VGA_display(u8 x, u8 y, u8 ch)
{
    unsigned int position = (y * VGA_TEXTMODE_WIDTH) + x;

    print_char_to_VGA_display_buffer(position, ch);
}

void print_string_to_VGA_display_buffer(u16 position, unsigned char* string, int string_size)
{
    for( int i = 0; i < string_size; i++)
    {
        print_char_to_VGA_display_buffer(position + i, string[i]);
    }
}
