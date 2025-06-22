#include "COM_port.h"
#include "instruction_wrappers.h"

// TODO: Hardcode it to 0x3F8 for now. Need to read it from BIOS Data Area
#define COM1_PORT (0x3F8)

void COM_port_TX(char data)
{
    // Wait for empty transmit buffer
    //while ((read_byte_from_IO_port(COM1_PORT) & 0x20) == 0);

    // Write data to transmit buffer
    write_byte_to_IO_port(COM1_PORT, data);
}
