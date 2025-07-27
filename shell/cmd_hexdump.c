/**
 * @file cmd_hexdump.c
 *
 * @brief Implements the basic behavior of a popular Unix command "hexdump"
 */

#include "cmd_hexdump.h"

#include "util.h"
#include "string.h"
#include "kernel_stdio.h"
#include "kernel_stddef.h"

#define BYTES_PER_LINE (16)

void execute__dump_data(int argc, char* argv[])
{
    if (argv == NULL)
    {
        LOG("ERROR: execute__dump_data() - received NULL arg");
        return;
    }

    // I've left it commented out in case I'll need it again
    //for( int i = 0; i < argc; i++ ) printf("\n argv[%i]: %s", i, argv[i]);
    //printf("\n");

    const unsigned char* const data = 0;
    int offset = 0;

    if( argc > 1)
    {
        offset = hex_to_long(argv[1]);
    }

    {
        // A small hack to continue dumping data from the last address of the previous incarnation,
        // if no address is specified
        static int previous_offset = 0;

        if( argc == 1)
        {
            offset = previous_offset + 0x80;
        }

        previous_offset = offset;
    }

    for (int row = 0; row < 8; row++)
    {
        {
            char line[] = "addrnumb: XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX   |Place_ASCII_text|";

            const char START_OF_ASCII_TEXT = 62; // Offset in line string

            // Generate address string
            // TODO: This func name is little bit missleading as the function can also generate base 10, not just base-16 hexadecimal
            long_to_hex(offset, line, 8, 16);

            int hex_line_offset = 10; // Offset in line string

            // Print hex values
            for(int hex_no = 0; hex_no < BYTES_PER_LINE; hex_no++)
            {
                unsigned char byte = data[offset+hex_no];
                
                //printf("\nLINE_OFFSET: %i", hex_line_offset);
                long_to_hex(byte, line + hex_line_offset, 2, 16);
                
                hex_line_offset += 3;
                
                if (hex_no == 7) hex_line_offset++;
            }

            // Print ASCII values
            for(int char_no = 0; char_no < BYTES_PER_LINE; char_no++)
            {
                char character = data[offset+char_no];

                // Replace control characters with a star and zero address with dot
                if (character == 0) character = '.';
                //if (character < 32) character = '*';

                line[START_OF_ASCII_TEXT + char_no] = character;
            }

            kernel_println(line);
        }

        offset += BYTES_PER_LINE;
    }

    kernel_println("");
}
