#include "COM_port.h"
#include "instruction_wrappers.h"

// TODO: Hardcode it to 0x3F8 for now. Need to read it from BIOS Data Area
#define COM1_PORT (0x3F8)

#define COM1_LSR_REG  (COM1_PORT + 5)
#define LSR_DATA_READY 0x01

void COM_port_TX(char data)
{
    // Wait for empty transmit buffer
    //while ((read_byte_from_IO_port(COM1_PORT) & 0x20) == 0);

    // Write data to transmit buffer
    write_byte_to_IO_port(COM1_PORT, data);

    // Add LF after CR (otherwise we overwrite the prompt)
    // TODO: This shouldn't be here, but will stay here until I fix the terminal code
    if (data == 13) write_byte_to_IO_port(COM1_PORT, 10);
}

char COM_port_RX(void)
{
    u8 line_status = read_byte_from_IO_port(COM1_LSR_REG);

#if 0
    {
        char code_str[15] = "AbcdAbcd";

        long_to_hex(line_status, code_str, 8, 16);
        //long_to_hex(val & ~0x80, code_str, 8, 16);

        kernel_println(code_str);
    }
#endif

    if ((line_status & LSR_DATA_READY) == 0)
    {
        return 0xff;
    }

    return read_byte_from_IO_port(COM1_PORT);
}
