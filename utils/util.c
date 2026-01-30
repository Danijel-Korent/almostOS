/*******************************************************************************
 *                                     UTIL                                    *
 *******************************************************************************/

#include "util.h"

#include "string.h"
#include "kernel_stdio.h" // TODO: We will need to add some LOG_safe() variant for
                         //       for usage in util functions


/*******************************************************************************
 *                           Memory related functions                          *
 *******************************************************************************/

// QTODO: add bool type as int. Add 0 as flase, 1 true
int memory_is_equal(const u8* const mem1, int mem1_size, u8* const mem2, int mem2_size)
{
    if (mem1 == NULL || mem2 == NULL) return 0; //QTODO: Log an error

    if (mem1_size != mem2_size) return 0;

    int is_equal = 1;

    for (int i = 0; i < mem1_size; i++)
    {
        if (mem1[i] != mem2[i])
        {
            is_equal = 0;
        }
    }

    return is_equal;
}

void mem_copy(u8* const destination, int destination_size, const u8* const source, int source_size)
{
    if (destination == NULL || source == NULL) return; //QTODO: Log an error

    int size = source_size;

    if (source_size > destination_size) size = destination_size;

    for (int i = 0; i < size; i++)
    {
        destination[i] = source[i];
    }
}

void* memset(void* ptr, int value, unsigned int num)
{
    unsigned char* p = (unsigned char*)ptr;
    for (unsigned int i = 0; i < num; i++)
    {
        p[i] = (unsigned char)value;
    }
    return ptr;
}


/*******************************************************************************
 *                         Number conversion functions                         *
 *******************************************************************************/

char byte_to_hexchar(unsigned char byte) // TODO: Replace with u/s typedef
{
    if (byte > 15)
    {
        // Circular dependency :(
        //
        // By calling LOG() function can end-up in a state similar to endless recursion since LOG() is calling again
        // byte_to_hexchar() to output CPU timestamp
        //
        // TODO:
        //      Need to decouple the reception and outputing of the LOG messages, so that byte_to_hexchar() don't get
        //      called in the LOG()
        //LOG("ERROR: byte_to_hexchar() - Input too big");

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

long int hex_to_long(const char* str) // TODO: Replace with u/s typedef
{
    if (str == NULL)
    {
        LOG("ERROR: hex_to_long() - received NULL arg");
        return 0;
    }

    long int result = 0;
    long int multiplier = 1;

    int index = strlen_unsafe(str) - 1;

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

// Maybe this should be called long_to_string()
// This func does not null-terminate. Should it?
void long_to_hex(long int number, char * string_buffer, int string_buffer_len, unsigned char base) // TODO: Replace with u/s typedef
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


int atoi(const char* str)
{
    if (str == NULL)
    {
        LOG("ERROR: hex_to_long() - received NULL arg");
        return 0;
    }

    long int result = 0;
    long int multiplier = 1;

    int index = strlen_unsafe(str) - 1;

    while( index >= 0)
    {
        int chr_value = 0;
        char chr = str[index];

        index--;

        if (chr >= '0' && chr <= '9')
        {
            chr_value = chr - '0';
        }
        else
        {
            LOG("WARNING: hex_to_long() - inappropriate character");
            continue; // Don't update multiplier and result
        }

        result += chr_value * multiplier;

        multiplier = multiplier * 10;
    }

    //printf("%s calculated to: %i \n", str, result);

    return result;
}
