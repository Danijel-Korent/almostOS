#include "string.h"

#include "util.h"
#include "kernel_stdio.h"


// TODO: Add explicit null-terminator
void append_string(u8* const destination, u32 destination_size, const u8* const source, u32 source_size)
{
    if (destination == NULL || source == NULL) return; //QTODO: Log an error

    u8 *end_of_first_string = destination;

    while (*end_of_first_string != 0 || (end_of_first_string-destination) >= destination_size)
    {
        end_of_first_string++;
    }

    u32 space_left_in_buffer = (destination + destination_size) - end_of_first_string;

    mem_copy(end_of_first_string, space_left_in_buffer-1, source, source_size);
}

/**
 * @brief Removes leading and trailing white-space
 *
 * @param input_string String to be trimmed
 * @return u8* String that is now trimmed
 *
 * @note The trimming is done 'in-place', meaning returned trimmed string is
 *       in the same memory space in which the original string was
 * @note For string allocated on heap, take care that pointer address can
 *       move forwad because of trimming of leading whitespace
 */
u8* trim_string(u8* input_string)
{
    if (input_string == NULL)
    {
        LOG("ERROR: trim_string() - received NULL argument!");
        return NULL;
    }

    u8 *trimmed_string = input_string;

    // Trim leading whitespace
    // Do it by just moving pointer along the orignal input string
    // util the first non whitespace symbol
    for(; *trimmed_string == ' '; trimmed_string++);

    // Trim trailing whitespace
    // Do it by null-terminating from end of string until
    // stumble upon non-whitespace char
    int len = strlen_unsafe(trimmed_string);

    while(len > 0)
    {
        if(trimmed_string[len -1] != ' ') break;

        // If it is whitespace, nullterminate it
        trimmed_string[len -1] = 0;

        len--;
    }

    return trimmed_string;
}

// TODO: Oh man how many strlen implementations do I have??

// TODO: Move this generic helper functions for length and trimming into a common file for helper functions
// TODO: Add postfix "_unsafe" to all unbounded functions
u32 strlen_unsafe(const u8* input_string)
{
    if (input_string == NULL) return 0;

    int i = 0;

    //for(i = 0; input_string[i] != 0; i++)
    while (input_string[i] != 0)
    {
        i++;

        if (i > 100) // TODO: Move this hardcoded number into a define
        {
            LOG("ERROR: strlen() smashed protection limit!");
            return 0;
        }
    }

    return i;
}

// TODO: Rename similar as get_string_size()
// Without LOG() to avoid circular calls
u32 strlen_unsafe_logless(const u8* input_string)
{
    if (input_string == NULL) return 0;

    u32 i = 0;

    //for(i = 0; input_string[i] != 0; i++)
    while (input_string[i] != 0)
    {
        i++;

        if (i > 100) // TODO: Move this hardcoded number into a define
        {
            //LOG("ERROR: strlen() smashed protection limit!");
            return 0;
        }
    }

    return i;
}

u32 get_string_size(const u8* const buffer_ptr, u32 buffer_size)
{
    if (buffer_ptr == NULL) return 0; //QTODO: Log an error

    const u8 *end_of_first_string_ptr = buffer_ptr;

    while (*end_of_first_string_ptr != 0 || (end_of_first_string_ptr-buffer_ptr) >= buffer_size)
    {
        end_of_first_string_ptr++;
    }

    return end_of_first_string_ptr - buffer_ptr;
}
