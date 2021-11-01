
// This needs to be compiled with -nostartfiles and -nostdlib since standard lib is not available

/*

    TODO NEXT:
        - Fix all compiler warnings
        - integrate the hexdump impl. from the FAT project
        - Read the information from the Multiboot
            -> https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Machine-state
        - FEATURE: Implement support for the CPUID
        - FEATURE: Implement timer support
        - Add logging for malloc() memory usage

        - Add drivers folder
        - Move to drivers folder
            - poors_man_keyboard_driver.c
            - poors_man_VGA_driver.c
            - add folder integration with driver_callbacks.c
        - Add folder user_applications
        - move terminal to "user_applications"
            - Rename terminal.c to poors_man_terminal
            - Rename shell.c to poors_man_shell.c
        - Delete old terminal code
        - Add headers for log and user terminal
        - Enable Wall i Werror
        - Transform compile_and_run.sh into makefile
        - Rename heap_malloc() to binned_mempool_malloc() / mempool_malloc()
        - Add a script for installing all required APT packages
            - Also add in to description / readme

        - Change GCC system include path to this folder or just disable it

        - Add process handler with 3 function pointers:
            - init()
            - service()
            - on_event()
            - standard input handler (stdin)
            - standard output handler (stdout)

        - Use pool allocator to implement binned heap allocator (check if it's already done)
        - Integrate textbuffer into terminal
        - Implement checks for buffer overflow
        - Move textbuffer into seperate file


    TODO LIST:
        - BUG:     Keyboard output is printed on key release instead on key press
        - TODO:    Create header for equivalent of stdint.h types
        - TODO:    Split kernel_main.c into multiple files
        - FEATURE: Implement basic terminal functionality
        - FEATURE: Integrate shell and FAT driver from "FAT-filesystem-driver" repo
        - FEATURE: Implement logging facilities
        - FEATURE: Implement bucket heap allocator
        - FEATURE: Implement serial/UART driver
        - FEATURE: Implement timer support
        - FEATURE: Implement support for the CPUID
*/

/*

 *******************************************************************************
 *                                 General IO                                  *
 *******************************************************************************

Legacy IBM PC:

    http://www.cs.cmu.edu/~ralf/files.html (famous Ralf Brown's interrupt list)
    http://opensecuritytraining.info/IntroBIOS_files/Day1_04_Advanced%20x86%20-%20BIOS%20and%20SMM%20Internals%20-%20IO.pdf


 *******************************************************************************
 *                              Memory Mapped IO                               *
 *******************************************************************************

    https://www.programmersought.com/article/19562113636/
    https://wiki.qemu.org/images/f/f6/PCIvsPCIe.pdf
    https://resources.infosecinstitute.com/topic/system-address-map-initialization-in-x86x64-architecture-part-1-pci-based-systems/
    https://resources.infosecinstitute.com/topic/system-address-map-initialization-x86x64-architecture-part-2-pci-express-based-systems/


 *******************************************************************************
 *                                 x86 IO bus                                  *
 *******************************************************************************

x86 IO BUS - General technical descriptions:

    http://bochs.sourceforge.net/techspec/PORTS.LST
    http://www.cs.cmu.edu/~ralf/files.html (PartD -> port.a, port.b, port.c)



Legacy IBM PC IO PORTS (XT, AT and PS/2 I/O port addresses)

    (serial ports, parallel ports, PS/2 keyboard, floppy, CMOS, ??)

    AT/PS2 Controler:
            - http://www.s100computers.com/My%20System%20Pages/MSDOS%20Board/PC%20Keyboard.pdf
            - http://helppc.netcore2k.net/hardware/8042
            - https://www.tayloredge.com/reference/Interface/atkeyboard.pdf
            - http://www.osdever.net/papers/view/wout-mertens-guide-to-keyboard-programming-v1.1-complete
            - http://www.osdever.net/papers/view/ibm-pc-keyboard-information-for-software-developers

    Serial    - https://sysplay.in/blog/pdfs/uart_pc16550d.pdf
    EGA     - http://www.minuszerodegrees.net/oa/OA%20-%20IBM%20Enhanced%20Graphics%20Adapter.pdf
    VGA        - https://wiki.osdev.org/VGA_Hardware
            - https://wiki.osdev.org/VGA_Resources
            - https://web.stanford.edu/class/cs140/projects/pintos/specs/freevga/vga/vga.htm
            - http://www.osdever.net/FreeVGA/vga/portidx.htm

MODERN STANDARDIZED

    IDE/ATAPI
    USB (UHCI, EHCI, XHCI and OHCI)


PORT LIST:

    0x0000-0x001F    The first legacy DMA controller, often used for transfers to floppies.
    0x0020-0x0021    The first Programmable Interrupt Controller
    0x0022-0x0023    Access to the Model-Specific Registers of Cyrix processors.
    0x0040-0x0047    The PIT (Programmable Interval Timer)
    0x0060-0x0064    The "8042" PS/2 Controller or its predecessors, dealing with keyboards and mice.
    0x0070-0x0071    The CMOS and RTC registers
    0x0080-0x008F    The DMA (Page registers)
    0x0092           The location of the fast A20 gate register
    0x00A0-0x00A1    The second PIC
    0x00C0-0x00DF    The second DMA controller, often used for soundblasters
    0x00E9           Home of the Port E9 Hack. Used on some emulators to directly send text to the hosts' console.
    0x0170-0x0177    The secondary ATA harddisk controller.
    0x01F0-0x01F7    The primary ATA harddisk controller.
    0x0278-0x027A    Parallel port
    0x02F8-0x02FF    Second serial port
    0x03B0-0x03DF    The range used for the IBM VGA, its direct predecessors, as well as any modern video card in legacy mode.
    0x03F0-0x03F7    Floppy disk controller
    0x03F8-0x03FF    First serial port

    http://bochs.sourceforge.net/techspec/PORTS.LST
    http://www.cs.cmu.edu/~ralf/files.html (PartD -> port.a, port.b, port.c)

 *******************************************************************************
 *                                   RANDOM                                    *
 *******************************************************************************

VESA VBE:
    - http://www.13thmonkey.org/documentation/Graphics/vbe3.pdf
    - http://www.neuraldk.org/cgi-bin/document.pl?djgppGraphics

Terminal escape code:
    - https://en.wikipedia.org/wiki/ANSI_escape_code ANSI X3.64 (ISO 6429)
    - http://ascii-table.com/ansi-escape-sequences-vt-100.php

*/

/*******************************************************************************
 *                                 Includes                                    *
 *******************************************************************************/

//#include <stdlib.h>  // Not available yet
//#include <stdio.h>   // Not available yet

#include "kernel_stddef.h"
#include "terminal.h"

/*******************************************************************************
 *                     Local defines, structs and typdefs                      *
 *******************************************************************************/

/*******************************************************************************
 *                              Global variables                               *
 *******************************************************************************/

/*******************************************************************************
 *                                Declarations                                 *
 *******************************************************************************/
void print_char_to_VGA_display_buffer(unsigned int position, unsigned char ch);
void print_char_to_VGA_display(unsigned int x, unsigned y, unsigned char ch);
void print_string_to_VGA_display_buffer(int position, unsigned char* string, int string_size);

void keyboard_driver_poll(void);

void terminal__init(void);
void terminal__print_string(unsigned char *string);
void terminal__render_to_VGA_display(void);

// Not really a heap allocator but a pool allocator, but will be turned into heap
void* heap_malloc(int size);
void heap_free(void* pointer);

bool run_textbox_unittests(void);
int run_unittests_stack(void);
int run_unittests_heap_allocator(void);

// Functions implemented startup.asm
// TODO: move both implementation and declaration into seperate files
int test_func(int base, int multiplier, int adder);
unsigned char read_byte_from_IO_port( unsigned short port_address);


/*******************************************************************************
 *                                    STUBS                                    *
 *******************************************************************************/

void* malloc(int size)
{
    return heap_malloc(size);
}

void free(void* pointer)
{
    heap_free(pointer);
}


// These functions are not available yet
int puts(const char *string)
{
    return 0;
}

int putchar ( int character )
{
    return 0;
}

int printf ( const char * format, ... )
{
    return 0;
}


/*******************************************************************************
 *                                  Functions                                  *
 *******************************************************************************/

terminal_contex_t shell_terminal;
terminal_contex_t log_terminal;

void LOG(const unsigned char* const message)
{
    terminal_printline(&log_terminal, message);
}

void stdandard_println(const unsigned char* const message)
{
    terminal_printline(&shell_terminal, message);
}

void kernel_c_main( void )
{
    unsigned char hello_msg[] = "Hello from C code!";
    print_string_to_VGA_display_buffer(400, hello_msg, sizeof(hello_msg)-1);

    //unsigned char shell_header[] = "-----------------------------------/ Shell /-----------------------------------";
    unsigned char shell_header[] = "-----------------------------------/ SHELL /-----------------------------------";
    print_string_to_VGA_display_buffer(560, shell_header, sizeof(shell_header)-1);

    //unsigned char klog_header[] = "--------------------------------/ Kernel log /---------------------------------";
    unsigned char klog_header[] = "--------------------------------/ KERNEL LOG /---------------------------------";
    print_string_to_VGA_display_buffer(1360, klog_header, sizeof(klog_header)-1);

    //int test = test_func(34, 2);

    //print_char_to_VGA_display_buffer(640, test_func(34, 2, 1));
    //print_char_to_VGA_display_buffer(642, test_func(35, 2, 1));
    //print_char_to_VGA_display_buffer(644, test_func(36, 2, 1));

    //terminal__init();

    // New terminal
    terminal_init(&shell_terminal,  8, 8, ENABLE_INPUT_LINE);
    terminal_init(&log_terminal,   18, 5, DISABLE_INPUT_LINE);

#if 0
    terminal_printline(&shell_terminal, "Linija broj  1");
    terminal_printline(&shell_terminal, "Linija broj  2");
    terminal_printline(&shell_terminal, "Linija broj  3");
    terminal_printline(&shell_terminal, "Linija broj  4");
    terminal_printline(&shell_terminal, "Linija broj  5");
    terminal_printline(&shell_terminal, "Linija broj  6");
    terminal_printline(&shell_terminal, "Linija broj  7");
    terminal_printline(&shell_terminal, "Linija broj  8");
    terminal_printline(&shell_terminal, "Linija broj  9");
    terminal_printline(&shell_terminal, "Linija broj 10");
    terminal_printline(&shell_terminal, "Linija broj 11");
    terminal_printline(&shell_terminal, "Linija broj 12");
    terminal_printline(&shell_terminal, "Linija broj 13");
    terminal_printline(&shell_terminal, "Linija broj 14");
    terminal_printline(&shell_terminal, "Linija broj 15");
    terminal_printline(&shell_terminal, "Linija broj 16");
    terminal_printline(&shell_terminal, "Linija broj 17");
    terminal_printline(&shell_terminal, "Linija broj 18");
    terminal_printline(&shell_terminal, "Linija broj 19");
    terminal_printline(&shell_terminal, "Linija broj 20");
    terminal_printline(&shell_terminal, "Linija broj 21");
    terminal_printline(&shell_terminal, "Linija broj 22");
    terminal_printline(&shell_terminal, "Linija broj 23");
    terminal_printline(&shell_terminal, "broj 24");
    terminal_printline(&shell_terminal, "broj 25");
    terminal_printline(&shell_terminal, "broj 26");
    terminal_printline(&shell_terminal, "broj 27");
#endif

    // QTODO: this repeating code needs a function of its own
    if (run_unittests_stack() == 0)
    {
        LOG("Stack unittests: PASSED");
    }
    else
    {
        LOG("Stack unittests: FAILED");
    }

    if (run_unittests_heap_allocator() == 0)
    {
        LOG("Memory allocator unittests: PASSED");
    }
    else
    {
        LOG("Memory allocator unittests: FAILED");
    }

    if (run_textbox_unittests())
    {
        LOG("Textbuffer unittests: PASSED");
    }
    else
    {
        LOG("Textbuffer unittests: FAILED");
    }

    while(1)
    {
        keyboard_driver_poll();
    }
}


/*******************************************************************************
 *                     stack declarations and definitions                      *
 *******************************************************************************/

typedef struct stack_handle_tag
{
    //int element_size; // Not used because implementation is harcoded to 1byte values
    int buffer_size;
    unsigned char* buffer_start;
    unsigned char* stack_pointer;
} stack_handle_t;

bool stack_init(stack_handle_t* const handle, unsigned char* const buffer, unsigned int buffer_size);
bool stack_push(stack_handle_t* const handle, unsigned char value);
bool stack_pop(stack_handle_t* const handle, unsigned char* const value);


/*******************************************************************************
 *                            Binned heap allocator                            *
 *  *actually currently just a pool memory allocator, but we will get there :3 *
 *******************************************************************************/

struct heap_bin_contex
{
    int block_size;
    int block_count;
    int buffer_size;
    unsigned char* buffer_start;
    stack_handle_t* free_blocks_stack;
};

static struct heap_bin_contex heap_bin;

#define BIN_BLOCK_COUNT    (3)
#define BIN_BLOCK_SIZE     (128)
#define BIN_RED_ZONE_SIZE  (32)

bool init_heap_memory_allocator(void)
{
    // Setup the buffer for heap storage
    {
        static unsigned char heap_buffer[BIN_BLOCK_COUNT*(BIN_BLOCK_SIZE+BIN_RED_ZONE_SIZE)];

        heap_bin.block_count = BIN_BLOCK_COUNT;
        heap_bin.block_size  = BIN_BLOCK_SIZE;

        heap_bin.buffer_start = heap_buffer;
        heap_bin.buffer_size  = sizeof(heap_buffer);

        if (heap_bin.block_count * (heap_bin.block_size + BIN_RED_ZONE_SIZE) > heap_bin.buffer_size)
        {
            return false;
        }
    }

    // Setup the stack for free blocks
    {
        static stack_handle_t free_blocks_stack;
        static unsigned char stack_buffer[BIN_BLOCK_COUNT];

        if ( ! stack_init(&free_blocks_stack, stack_buffer, sizeof(stack_buffer)))
        {
            return false;
        }

        // Populate free blocks list
        for (int i = 0; i < BIN_BLOCK_COUNT; i++)
        {
            if ( ! stack_push(&free_blocks_stack, i))
            {
                return false;
            }
        }

        heap_bin.free_blocks_stack = &free_blocks_stack;
    }

    return true;
}

void* heap_malloc(int size)
{
    if (size == 0) return NULL; // QTODO: Log an error

    if (size > heap_bin.block_size)
    {
        return NULL; // QTODO: Log an error
    }

    unsigned char index = 0;
    void* calculated_address = NULL;

    if (stack_pop(heap_bin.free_blocks_stack, &index))
    {
        if (index >= heap_bin.block_count)
        {
            // QTODO: Log an error
        }
        else
        {
            unsigned int offset = index * (heap_bin.block_size + BIN_RED_ZONE_SIZE);

            calculated_address = (void*)(heap_bin.buffer_start + offset);
        }
    }

    return calculated_address;
}

void heap_free(void* pointer)
{
    if (pointer == NULL) return; // QTODO: Log an error

    unsigned char index = 0;
    unsigned int offset = 0;

    offset = (unsigned char*)pointer - heap_bin.buffer_start;

    index = offset / (heap_bin.block_size + BIN_RED_ZONE_SIZE);

    if ( ! stack_push(heap_bin.free_blocks_stack, index))
    {
        // Log an error
    }
}

int run_unittests_heap_allocator(void)
{
    int failed_test_counter = 0;

    if (init_heap_memory_allocator())
    {
        // Must store all allocated addresses for freeing later
        // -1 because buffer filled with 0xFF will be stored in separate var
        void* returned_addresses[BIN_BLOCK_COUNT -1] = {0};

        unsigned char* buffer_with_pattern_0xff = NULL;

        // For this implementation this should not corrupt allocator
        heap_free(NULL);

        // Test for size limitation
        if (heap_malloc(BIN_BLOCK_SIZE + 1) != NULL)
        {
            failed_test_counter++;
        }

        // Allocate first block and fill it with 0xFF pattern
        // TODO: move this allocation between two other allocations
        {
            buffer_with_pattern_0xff = heap_malloc(BIN_BLOCK_SIZE);

            if (buffer_with_pattern_0xff == NULL)
            {
                failed_test_counter++;
            }

            for (int i = 0; i < BIN_BLOCK_SIZE; i++)
            {
                buffer_with_pattern_0xff[i] = 0xff;
            }
        }

        // Allocate all the rest of available blocks and color them with 0x11
        {
            for (int x = 0; x < BIN_BLOCK_COUNT -1; x++)
            {
                unsigned char* buffer = heap_malloc(BIN_BLOCK_SIZE);

                if (buffer != NULL)
                {
                    for (int i = 0; i < BIN_BLOCK_SIZE; i++)
                    {
                        buffer[i] = 0x11;
                    }
                }
                else
                {
                    failed_test_counter++;
                }

                returned_addresses[x] = buffer;
            }
        }

        // All blocks are allocated and this malloc should fail
        {
            unsigned char* buffer = heap_malloc(BIN_BLOCK_SIZE);

            if (buffer != NULL)
            {
                failed_test_counter++;
            }
        }

        // Free one block and they try new allocation and fill the memory
        {
            void* pointer = returned_addresses[0];

            heap_free(pointer);

            unsigned char* buffer = heap_malloc(5);

            if (buffer == NULL)
            {
                failed_test_counter++;
            }

            returned_addresses[0] = buffer;
        }

        // Free all memory blocks except the one with 0xFF pattern
        {
            for (int i = 0; i<sizeof(returned_addresses); i++)
            {
                heap_free(returned_addresses[i]);
            }
        }

        // Check that the first allocation still holds the 0xFF pattern
        {
            for (int i = 0; i < BIN_BLOCK_SIZE; i++)
            {
                if (buffer_with_pattern_0xff[i] != 0xff)
                {
                    failed_test_counter++;
                }
            }

            heap_free(buffer_with_pattern_0xff);
        }
    }
    else
    {
        failed_test_counter++;
    }

    return failed_test_counter;
}

/*******************************************************************************
 *                         simple stack implementation                         *
 *******************************************************************************/

// Add size parameter and check if buffer is big enough for it??
bool stack_init(stack_handle_t* const handle, unsigned char* const buffer, unsigned int buffer_size)
{
    if (handle == NULL || buffer == NULL || buffer_size == 0) return false; // Log error

    handle->buffer_start = buffer;
    handle->buffer_size = buffer_size;

    handle->stack_pointer = buffer;

    return true;
}

bool stack_push(stack_handle_t* const handle, unsigned char value)
{
    if (handle == NULL) return false; // Log error

    if (handle->stack_pointer > (handle->buffer_start + handle->buffer_size -1))
    {
        return false; // The stack is full
    }

    *handle->stack_pointer = value;
     handle->stack_pointer++;

     return true;
}

bool stack_pop(stack_handle_t* const handle, unsigned char* const value)
{
    if (handle == NULL || value == NULL) return false; // Log error

    if (handle->stack_pointer == handle->buffer_start)
    {
        return false; // The stack is empty
    }

    handle->stack_pointer--;
    *value = *handle->stack_pointer;

    // Clean the garbage that is left behind. Not necessary, but to make potential bugs more predictable/reproducible
    *handle->stack_pointer = 0;

    return true;
}


int run_unittests_stack(void)
{
    int failed_test_counter = 0;

    stack_handle_t handle;
    static unsigned char buffer[3];

    // TODO: test null pointer arguments

    if (stack_init(&handle, buffer, sizeof(buffer)))
    {
        unsigned char returned_val = 0;

        if (stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is empty and should not return value
        }

        if ( ! stack_push(&handle, 1))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if ( ! stack_push(&handle, 2))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if ( ! stack_push(&handle, 3))
        {
            failed_test_counter++; // Stack is NOT empty and should successfully push the value
        }

        if (stack_push(&handle, 4))
        {
            failed_test_counter++; // Stack is FULL and should fail to push the value
        }


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 3) failed_test_counter++; // Wrong value


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 2) failed_test_counter++; // Wrong value


        if ( ! stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is NOT empty and should return value
        }

        if (returned_val =! 1) failed_test_counter++; // Wrong value

        if (stack_pop(&handle, &returned_val))
        {
            failed_test_counter++; // Stack is empty and should not return value
        }
    }

    return failed_test_counter;
}

/*******************************************************************************
 *                               Textbuffer class                              *
 *******************************************************************************/

// A circular text buffer that is aware of lines and textbox size

#define TERMINAL_WIDTH (80)
#define TEXTBUFFER_NO_LINES (50)

typedef struct textbuffer_handler_tag
{
    unsigned char buffer[TEXTBUFFER_NO_LINES * TERMINAL_WIDTH + 1]; // +1 because one location is always unused (buffer_end)
    unsigned int  buffer_start;
    unsigned int  buffer_end;
} textbuffer_handle_t;

typedef int textbuffer_iterator_t;

void textbuffer__add_char(textbuffer_handle_t* const handle, unsigned char character_to_add);



void textbuffer__init(textbuffer_handle_t* const handle)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return; // QTODO: return error code
    }

    handle->buffer_start = 0;
    handle->buffer_end   = 0;

    // TODO: clear the buffer
    // TODO: log init call
}

unsigned int textbuffer__get_content_length(textbuffer_handle_t* const handle)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return 0; // QTODO: return error code
    }

    unsigned int result = 0;
    unsigned int buffer_end   = handle->buffer_end;
    unsigned int buffer_start = handle->buffer_start;

    if (buffer_start < buffer_end) // No wrap-around
    {
        result = buffer_end - buffer_start;
    }
    else if (buffer_start > buffer_end) // wrap-around happend
    {
        result = (sizeof(handle->buffer) - buffer_start) + buffer_end;
    }
    else // if end == start
    {
        result = 0;
    }

    return result;
}

void textbuffer__increment_iterators_on_bytewrite(textbuffer_handle_t* const handle)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return; // QTODO: return error code
    }

    unsigned int buffer_size    = sizeof(handle->buffer);

     handle->buffer_end++;
     handle->buffer_end = handle->buffer_end % buffer_size; // Wrape-around if neccessery

     if(handle->buffer_end == handle->buffer_start)
     {
         handle->buffer_start += TERMINAL_WIDTH; // Remove a whole line from the beginning of the buffer
         handle->buffer_start = handle->buffer_start % buffer_size; // Wrape-around if neccessery

     }
}

void textbuffer__skip_to_next_line(textbuffer_handle_t* const handle)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return; // QTODO: return error code
    }

    unsigned int last_row_length = textbuffer__get_content_length(handle) % TERMINAL_WIDTH;

    if (last_row_length == 0)
    {
        return;
    }

    unsigned int num_of_chars_to_skip = TERMINAL_WIDTH - last_row_length;

    while (num_of_chars_to_skip > 0)
    {
        textbuffer__add_char(handle, ' ');

        num_of_chars_to_skip--;
    }
}

void textbuffer__add_char(textbuffer_handle_t* const handle, unsigned char character_to_add)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return; // QTODO: return error code
    }

    // TODO: escape characters should be handled by terminal code
    if (character_to_add == '\n')
    {
        // TODO: circular call, but will be moved out anyway
        textbuffer__skip_to_next_line(handle);
    }
    else
    {
        handle->buffer[handle->buffer_end] = character_to_add;

        textbuffer__increment_iterators_on_bytewrite(handle);
    }
}

void textbuffer__add_string(textbuffer_handle_t *handle, const unsigned char* const string)
{
    if (handle == NULL || string == NULL) return; //QTODO: Log an error

    //puts("\nAdding string: ");

    const unsigned char *string_itr = string;

    while (*string_itr != 0)
    {
        //if (*string_itr == '\n') putchar('*');
        //else putchar(*string_itr);

        textbuffer__add_char(handle, *string_itr++);
    }

    //print_handle(handle);
}

unsigned int textbuffer__get_line_cout(textbuffer_handle_t* const handle)
{
    if (handle == NULL)
    {
        //QTODO: log error mesage
        return 0; // QTODO: return error code
    }

    unsigned int content_length = textbuffer__get_content_length(handle);

    unsigned int result   = content_length / TERMINAL_WIDTH;
    unsigned int reminder = content_length % TERMINAL_WIDTH;

    if (reminder > 0)
    {
        result++;
    }

    return result;
}


// Iterator becomes invalid if textbuffer is modified after getting iterator
textbuffer_iterator_t textbuffer_get_iterator_at_line(textbuffer_handle_t* const handle, int line_num)
{
    if (handle == NULL || line_num >= TEXTBUFFER_NO_LINES)
    {
        //QTODO: log error mesage
        return -1; // QTODO: add enum for error code
    }

    unsigned int offset         = line_num * TERMINAL_WIDTH;
    unsigned int content_length = textbuffer__get_content_length(handle);

    if (offset > content_length)
    {
        //QTODO: log error mesage
        return -1; // QTODO: add enum for error code
    }


    // Calculate index from the offset
    textbuffer_iterator_t index = handle->buffer_start + offset;

    if (index > sizeof(handle->buffer))
    {
        index = index - sizeof(handle->buffer);
    }

    // TODO: assert that index is not bigger than buffer_start

    return index;
}

int textbuffer__get_next_memblock(textbuffer_handle_t* const handle, textbuffer_iterator_t *iterator, unsigned char** const buffer, int* const buffer_size)
{
    if (handle == NULL || iterator == NULL || buffer == NULL || buffer_size == NULL) //QTODO: Hardcoded error code
    {
        //QTODO: log error mesage
        return -1; // QTODO: add enum for error code
    }

    if (*iterator == -1 || *iterator == handle->buffer_end)
    {
        *buffer = NULL;
        *buffer_size = 0;
        return -1; // End of the iterator/buffer
    }

    unsigned int buffer_end   = handle->buffer_end;
    unsigned int buffer_start = *iterator;

    textbuffer_iterator_t next_iterator = 0;

    if (buffer_start < buffer_end) // No wrap-around
    {
        next_iterator = buffer_end;
        *buffer_size  = buffer_end - buffer_start;
    }
    else if (buffer_start > buffer_end) // wrap-around happend
    {
        next_iterator = 0;
        *buffer_size  = sizeof(handle->buffer) - buffer_start;
    }
    else // if end == start
    {
        *buffer_size = 0;
        *buffer      = NULL;
        return -1;
    }

    *buffer = handle->buffer + *iterator;

    *iterator = next_iterator;

    return 0;
}

unsigned int textbuffer__get_current_x(textbuffer_handle_t* const handle)
{
    unsigned int content_length = textbuffer__get_content_length(handle);

    return 0;
}


unsigned int textbuffer__get_current_y(textbuffer_handle_t* const handle)
{
    unsigned int content_length = textbuffer__get_content_length(handle);

    return 0;
}


/*******************************************************************************
 *                             TEXTBUFFER UNITTESTS                            *
 *******************************************************************************/

void print_handle(textbuffer_handle_t* const handle)
{
    if (handle == NULL) return; //QTODO: Log an error

    printf("\nBuffer start: %i", handle->buffer_start);
    printf("\nBuffer end:   %i\n", handle->buffer_end);
}

void print_buffer(const unsigned char* const buffer, int buffer_size)
{
    if (buffer == NULL) return; //QTODO: Log an error

    for (int y = 0; y < TEXTBUFFER_NO_LINES; y++)
    {
        for (int x = 0; x < TERMINAL_WIDTH; x++)
        {
            int offset = y * TERMINAL_WIDTH + x;

            if (offset >= buffer_size) return;

            signed char character = buffer[offset];

            if (character == ' ') character = '_';

            if (character < ' ')
            {
                if (character == 0)
                    character = 176;
                else
                    character = 178;
            }

            putchar(character);
        }
        printf("|\n");
    }
}

// QTODO: add bool type as int. Add 0 as flase, 1 true
int memory_is_equal(const unsigned char* const mem1, int mem1_size, const unsigned char* const mem2, int mem2_size)
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

void output_textbuffer_to_memory(textbuffer_handle_t* const handle, unsigned char* const memory, int memory_size, int* const received_data_size)
{
    if (handle == NULL || memory == NULL || received_data_size == NULL) return; //QTODO: Log an error

    unsigned char *buffer;
    int buffer_size;

    unsigned char *memory_iterator = memory;
    unsigned char *memory_end = memory + memory_size;


    textbuffer_iterator_t iter = textbuffer_get_iterator_at_line(handle, 0);

    while (textbuffer__get_next_memblock(handle, &iter, &buffer, &buffer_size) == 0)
    {
        for (int i = 0; i < buffer_size; i++)
        {
            if(memory_iterator != memory_end)
            {
                *memory_iterator = buffer[i];
                memory_iterator++;
            }
        }
    }

    *received_data_size = memory_iterator - memory;
}

bool check_test_case(textbuffer_handle_t* const handle, const unsigned char* const expected_result, int expected_result_size)
{
    if (handle == NULL || expected_result == NULL) return 0; //QTODO: Log an error

    bool test_case_passed = true;

    unsigned char receive_buffer[TEXTBUFFER_NO_LINES * TERMINAL_WIDTH] = {0};
    int received_data_size = 0;

    output_textbuffer_to_memory(handle, receive_buffer, sizeof(receive_buffer), &received_data_size);

    //print_buffer(receive_buffer, sizeof(receive_buffer));

    if (! memory_is_equal(receive_buffer, received_data_size, expected_result, expected_result_size))
    {
        test_case_passed = false;
    }

    if (test_case_passed)
    {
        puts("Test case passed!");
    }
    else
    {
        puts("\nReceived:");
        print_buffer(receive_buffer, sizeof(receive_buffer));

        puts("\nExpected:");
        print_buffer(expected_result, expected_result_size);

        puts("\n\nTest case FAILED!");
    }

    return test_case_passed;
}

void mem_copy(unsigned char* const destination, int destination_size, const unsigned char* const source, int source_size)
{
    if (destination == NULL || source == NULL) return; //QTODO: Log an error

    int size = source_size;

    if (source_size > destination_size) size = destination_size;

    for (int i = 0; i < size; i++)
    {
        destination[i] = source[i];
    }
}

void append_string(unsigned char* const destination, int destination_size, const unsigned char* const source, int source_size)
{
    if (destination == NULL || source == NULL) return; //QTODO: Log an error

    unsigned char *end_of_first_string = destination;

    while (*end_of_first_string != 0 || (end_of_first_string-destination) >= destination_size)
    {
        end_of_first_string++;
    }

    int space_left_in_buffer = (destination + destination_size) - end_of_first_string;

    mem_copy(end_of_first_string, space_left_in_buffer-1, source, source_size);
}

int get_string_size(const unsigned char* const buffer_ptr, int buffer_size)
{
    if (buffer_ptr == NULL) return 0; //QTODO: Log an error

    const unsigned char *end_of_first_string_ptr = buffer_ptr;

    while (*end_of_first_string_ptr != 0 || (end_of_first_string_ptr-buffer_ptr) >= buffer_size)
    {
        end_of_first_string_ptr++;
    }

    return end_of_first_string_ptr - buffer_ptr;
}

// TODO:
//      Test are flexible to changes to TEXTBUFFER_NO_LINES, but with is fixed to 80.
//      Not sure if it worth make it flexible since that would make test cases very hard to read
bool run_textbox_unittests(void)
{
    puts("\n\n !! Running Unittests !!");

    bool all_unittest_passed = true;

    textbuffer_handle_t test_handle;

    textbuffer__init(&test_handle);

    // Test adding text
    {
        textbuffer__add_string(&test_handle, "X");

        unsigned char expected_result[] = "X";

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }

    {
        textbuffer__add_string(&test_handle,  "000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000");
        textbuffer__add_string(&test_handle, "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111");

        unsigned char expected_result[] = "X000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000"
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          ;

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }

    {
        textbuffer__add_string(&test_handle, "2");

        unsigned char expected_result[] = "X000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000"
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          "2"
                                          ;

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }

    // Test new line handling
    {
        textbuffer__add_string(&test_handle, "2\n");

        unsigned char expected_result[] = "X000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000"
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          "22        ""          ""          ""          ""          ""          ""          ""          "
                                          ;

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }

    // Test that new line at end of line are ignored
    {
        textbuffer__add_string(&test_handle, "\n\n\n");

        unsigned char expected_result[] = "X000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000"
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          "22        ""          ""          ""          ""          ""          ""          ""          "
                                          ;

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }

    // Test filling the buffer to a full size
    {
        int lines_to_fill_whole_buffer = TEXTBUFFER_NO_LINES - 3;

        // Fill the textbuffer
        for (int i = 0; i < lines_to_fill_whole_buffer; i++)
        {
            textbuffer__add_string(&test_handle, "AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA");
        }

        // Construct the expected answer
        unsigned char expected_result[TEXTBUFFER_NO_LINES * TERMINAL_WIDTH +1] =
                                          "X000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000""0000000000"
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          "22        ""          ""          ""          ""          ""          ""          ""          "
                                          ;


        for (int i = 0; i < (lines_to_fill_whole_buffer); i++)
        {
            const char line_to_add[] = "AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA";

            append_string(expected_result, sizeof(expected_result), line_to_add, sizeof(line_to_add));
        }

        if( ! check_test_case(&test_handle, expected_result, sizeof(expected_result) - 1)) // -1 to exclude null terminator
        {
            all_unittest_passed = false;
        }
    }


    // Test wrap-around
    {
        textbuffer__add_string(&test_handle, "3");

        // Construct the expected answer
        unsigned char expected_result[TEXTBUFFER_NO_LINES * TERMINAL_WIDTH +1] =
                                          "1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111""1111111111"
                                          "22        ""          ""          ""          ""          ""          ""          ""          "
                                          ;

        int lines_to_fill_whole_buffer = TEXTBUFFER_NO_LINES - 3;

        for (int i = 0; i < (lines_to_fill_whole_buffer); i++)
        {
            const char line_to_add[] = "AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA";

            append_string(expected_result, sizeof(expected_result), line_to_add, sizeof(line_to_add));
        }

        append_string(expected_result, sizeof(expected_result), "3", 2);

        if( ! check_test_case(&test_handle, expected_result, get_string_size(expected_result, sizeof(expected_result))))
        {
            all_unittest_passed = false;
        }
    }

    // Another test for wrap-around
    {
        char string_to_add[] = "BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB""BBBBBBBBBB"
                               "BB"
                               ;
        textbuffer__add_string(&test_handle, string_to_add);


        // Construct the expected answer
        unsigned char expected_result[TEXTBUFFER_NO_LINES * TERMINAL_WIDTH +1] =
                                          "22        ""          ""          ""          ""          ""          ""          ""          "
                                          ;

        int lines_to_fill_whole_buffer = TEXTBUFFER_NO_LINES - 3;

        for (int i = 0; i < (lines_to_fill_whole_buffer); i++)
        {
            const char line_to_add[] = "AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA""AAAAAAAAAA";

            append_string(expected_result, sizeof(expected_result), line_to_add, sizeof(line_to_add));
        }

        append_string(expected_result, sizeof(expected_result), "3", 2);
        append_string(expected_result, sizeof(expected_result), string_to_add, sizeof(string_to_add));

        if( ! check_test_case(&test_handle, expected_result, get_string_size(expected_result, sizeof(expected_result))))
        {
            all_unittest_passed = false;
        }
    }

    return all_unittest_passed;
}


/*******************************************************************************
 *                            OLD Terminal functions                           *
 *******************************************************************************/

// TODO: All this data will be part of a structure/handle
//       to have multiple active terminals simultaneously
// QTODO: replace hardcoded numbers with defines
static unsigned char terminal__textbuffer[50*80];
//static unsigned int  terminal__textbuffer_start = 0;
static unsigned int  terminal__textbuffer_end = 0;


static unsigned int  terminal__window_start  =  8;
static unsigned int  terminal__window_length = 15;


void terminal__init(void)
{
#if 0
    terminal__textbuffer[0]   = 'T';
    terminal__textbuffer[80]  = 'E';
    terminal__textbuffer[160] = 'S';
    terminal__textbuffer[240] = 'T';
#endif

    terminal__print_string("Terminal initialized"); // lol
    // Place terminal from line 15 to 23

    // Text mode width: 80 chars
}

//void terminal__poll() // Is this needed? Probably all events will happen on keypress

void terminal__on_keypress(unsigned char key)
{
    // Process the keypress event

    // Update the screen
    terminal__render_to_VGA_display();
}

// TODO: will be local/static functionvoid
void terminal__render_to_VGA_display(void)
{
    unsigned char output_starting_line = 0;

    // Code for following the output of terminal
    // Calculate the starting line for outputing from the textbuffer if there is more text
    // to display than the size of the terminal window
    // TODO: And this is now offically a spaghetti code - but will be fixed when textbuffer
    // is seperated into its own class
    {
        if (terminal__textbuffer_end >= terminal__window_length*80) // QTODO: hardcoded 80
        {
            int num_of_lines = terminal__textbuffer_end / 80 + 1;

            output_starting_line = num_of_lines - terminal__window_length;
        }
    }

    // TODO: Replace hardcoded '80'
    for (int line = 0; line < terminal__window_length; line++)
    {
        for (int column = 0; column < 80; column++)
        {
            // QTODO: sooooo ugly, make it nicer!
            unsigned char output_char = terminal__textbuffer[(line+output_starting_line)*80+column];
            unsigned int  output_position = (line*80) + column + (terminal__window_start*80); // QTODO: hardcoded 80

            print_char_to_VGA_display(column, line+terminal__window_start, output_char);
        }
    }
}



void terminal__print_char(unsigned char char_to_output)
{
    // if char == '\n' go to next line
    if (char_to_output == '\n')
    {
        unsigned int current_column = terminal__textbuffer_end % 80; // TODO: Replace hardcoded literal

        // QTODO BUG: Space between current_column and the new line must be cleared with whitespaces,
        //            otherwise old data will be showed when this buffer will be circular

        terminal__textbuffer_end += 80 - current_column; // TODO: Replace hardcoded literal
    }
    else
    {
        terminal__textbuffer[terminal__textbuffer_end++] = char_to_output;
    }

}

void terminal__print_string(unsigned char *string)
{
    if (string == NULL) return; // TODO: log error

    while (*string != 0)
    {
        terminal__print_char(*string);

        string++;
    }

    terminal__render_to_VGA_display();
}

/*******************************************************************************
 *                             VGA Output functions                            *
 *******************************************************************************/

#define VGA_TEXTMODE_WIDTH  (80)
#define VGA_TEXTMODE_HEIGHT (25)

// TODO: Add functions for moving blinking cursor

void print_char_to_VGA_display_buffer(unsigned int position, unsigned char ch)
{
    static unsigned char* const VGA_RAM = (unsigned char*) 0x000B8000;

    position = position * 2; // One char takes 16 bits in VGA RAM (8bit char to display + 8bit for color)

    // QTODO: hardcoded color - add arguments for foreground and background color
    VGA_RAM[position]   = ch;
    VGA_RAM[position+1] = 0x13; // Blue + Cyan
    VGA_RAM[position+1] = 0x0A; // Black + Light green
    VGA_RAM[position+1] = 0x02; // Black + Green
}

void print_char_to_VGA_display(unsigned int x, unsigned int y, unsigned char ch)
{
    unsigned int position = (y * VGA_TEXTMODE_WIDTH) + x;

    print_char_to_VGA_display_buffer(position, ch);
}

// TODO: This was only used for testing and can be removed now
void print_string_to_VGA_display_buffer(int position, unsigned char* string, int string_size)
{
    for( int i = 0; i < string_size; i++)
    {
        print_char_to_VGA_display_buffer(position + i, string[i]);
    }
}


/*******************************************************************************
 *                         Poor man's keyboard driver                          *
 *******************************************************************************/

void event_on_keypress(unsigned char key)
{
    // Temporary test code
#if 0
    {
        static int i = 640;

        if (i >= 640+400) i = 640; // Display it from line 8 to line 13

        print_char_to_VGA_display_buffer(i, key);
        i++;
    }
#endif

    // Temporary test code
    {
        if (key > '0' && key < '6')
        {
            char string[] = "New terminal test string ";
            string[sizeof(string) -2] = key;

            terminal_printline(&shell_terminal, string);
        }
    }

    // TEMP
    //terminal__print_string("\nKey is pressed: ");
    //terminal__print_char(key);

    //terminal__on_keypress(key);

#if 0
    char string[] = "Key is pressed:  ";
    string[sizeof(string) -2] = key;
    terminal_printline(&shell_terminal, string);
#endif

    terminal_on_keypress(&shell_terminal, key);
    //LOG(string);
}


unsigned char convert_scancode_to_ASCII(unsigned char scan_code)
{

    // Quick and dirty solution for reading at least letters and numbers
    // Proper solution will need a lot more code to check/support scan code sets,
    // support upper/lower cases, control/alt/shift combinations...
    static unsigned char scancode_to_ASCII_table[128] =
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
    static unsigned char previous_scan_code = 0;

    // TODO: Before reading output, check if any data is actually pending
    unsigned char scan_code = read_byte_from_IO_port(0x60);

    if ((previous_scan_code != scan_code) && ((scan_code & 0x80) == 0x80))
    {
        unsigned char ASCII_code = convert_scancode_to_ASCII(scan_code);
        event_on_keypress(ASCII_code);
    }

    previous_scan_code = scan_code;
}

