/**
 * @file hexdump.c
 *
 * @brief Implements the basic behavior of a popular Unix command "hexdump"
 *
 *  TODO: Helper functions are generic and should be moved together to a common file like "util.c" or something
 */

#include "kernel_stddef.h"

int strlen(const char * input_string);
void LOG(const unsigned char* const message); // TODO: Create a header for this
void stdandard_println(const unsigned char* const message); // TODO: Create a header for this


static long int hex_to_long(const char* str)
{
    if (str == NULL)
    {
        LOG("ERROR: hex_to_long() - received NULL arg");
        return 0;
    }

    long int result = 0;
    long int multiplier = 1;

    int index = strlen(str) - 1;

    while( index >= 0)
    {
        int chr_value = 0;
        char chr = str[index];

        index--;

        if (chr >= '0' && chr <= '9')
        {
            chr_value = chr - '0';
        }
        else if (chr >= 'a' && chr <= 'f')
        {
            chr_value = chr - 'a' + 10;
        }
        else if (chr >= 'A' && chr <= 'F')
        {
            chr_value = chr - 'A' + 10;
        }
        else
        {
            LOG("WARNING: hex_to_long() - inappropriate character");
            continue; // Don't update multiplier and result
        }

        result += chr_value * multiplier;

        multiplier = multiplier * 16;
    }

    //printf("%s calculated to: %i \n", str, result);

    return result;
}

static char byte_to_hexchar(unsigned char byte)
{
    if (byte > 15)
    {
        LOG("ERROR: byte_to_hexchar() - Input too big");
        return 0;
    }
    
    if(byte < 10)
    {
        return '0' + byte;
    }
    else
    {
        return 'A' + byte - 10;
    }
}

// Maybe this should be called long_to_string()
// This func does not null-terminate. Should it?
void long_to_hex(long int number, char * string_buffer, int string_buffer_len, unsigned char base)
{
    if (string_buffer == NULL)
    {
        LOG("ERROR: long_to_hex() - received NULL arg");
        return;
    }

    int index = string_buffer_len - 1;
    
    while (index >= 0)
    {
        unsigned char reminder = 0;

        reminder = number % base;
        number   = number / base;

        string_buffer[index] = byte_to_hexchar(reminder);

        index--;
    }
    
    if (number != 0)
    {
        LOG("ERROR: long_to_hex() - String buffer too small for input number!");
    }
}

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
    
            stdandard_println(line);
        }

        offset += 16; // QTODO: Add name to magic number
    }
}
