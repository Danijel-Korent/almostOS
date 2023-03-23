/**
 * @file hexdump.c
 *
 * @brief Implements the basic behavior of a popular Unix command "hexdump"
 *
 *  TODO: Helper functions are generic and should be moved together to a common file like "util.c" or something
 */

#include "hexdump.h"

#include "util.h"
#include "string.h"
#include "kernel_stdio.h"
#include "kernel_stddef.h"


void execute__dump_data(int argc, char* argv[])
{
    if (argv == NULL)
    {
        LOG("ERROR: execute__dump_data() - received NULL arg");
        return;
    }

    //for( int i = 0; i < argc; i++ ) printf("\n argv[%i]: %s", i, argv[i]);
    //printf("\n");

    const unsigned char* const data = 0;
    int offset = 0;

    if( argc > 1)
    {
        // QTODO: Replace hardcoded '20' with a variable 1/2
        offset = hex_to_long(argv[1]);
    }

    {
        static int previous_offset = 0;

        if( argc == 1)
        {
            offset = previous_offset + 0x80;
        }

        previous_offset = offset;
    }

    for (int row = 0; row < 8; row++) // QTODO: Replace hardcoded '20' with a variable 2/2
    {
        {
            char line[] = "addrnumb: XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX   |Place_ASCII_test|";
    
            long_to_hex(offset, line, 8, 10);
            
            int hex_line_offset = 10;
            
            for(int hex_no = 0; hex_no < 16; hex_no++)
            {
                unsigned char byte = data[offset+hex_no];
                
                //printf("\nLINE_OFFSET: %i", hex_line_offset);
                long_to_hex(byte, line + hex_line_offset, 2, 16);
                
                hex_line_offset += 3;
                
                if (hex_no == 7) hex_line_offset++;
            }
            
            // Print ASCII values
            for(int char_no = 0; char_no < 16; char_no++)
            {
                char character = data[offset+char_no];
    
                // Replace control characters with a star and zero address with dot
                if (character == 0) character = '.';
                //if (character < 32) character = '*';
    
                line[62 + char_no] = character;
            }
    
            kernel_println(line);
        }

        offset += 16; // QTODO: Add name to magic number
    }
}
