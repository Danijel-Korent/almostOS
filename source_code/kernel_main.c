
// This needs to be compiled with -nostartfiles and -nostdlib since standard lib is not available

/*

    TODO NEXT:
        - Rename heap_malloc() to binned_mempool_malloc() / mempool_malloc()

        - Read the information from BIOS Data Area

        - Add logging for malloc() memory usage
            - Print the memory allocator usage stats after every (de)allocation


        - Read the information from the Multiboot
            -> https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Machine-state
        - FEATURE: Implement support for the CPUID
        - FEATURE: Implement timer support

        - Add drivers folder
        - Move to drivers folder
            - poors_man_keyboard_driver.c
            - poors_man_VGA_driver.c
            - add folder integration with driver_callbacks.c
        - Add folder tools
        - move terminal to "tools"
            - Rename terminal.c to poors_man_terminal
            - Rename shell.c to poors_man_shell.c
        - Add headers for log and user terminal
        - Enable Wall i Werror

        - Add a script for installing all required APT packages
            - Also add in to description / readme

        - Change GCC system include path to this folder or just disable it

        - Add process handler with 3 function pointers:
            - init()
            - service()
            - on_event()
            - standard input handler (stdin)
            - standard output handler (stdout)

        - Implement checks for buffer overflow


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

#include "kernel_stddef.h"
#include "terminal.h"
#include "instruction_wrappers.h"

#include "poors_man_VGA_driver.h"
#include "poors_man_keyboard_driver.h"


/*******************************************************************************
 *                     Local defines, structs and typdefs                      *
 *******************************************************************************/

/*******************************************************************************
 *                              Global variables                               *
 *******************************************************************************/

/*******************************************************************************
 *                                Declarations                                 *
 *******************************************************************************/

void keyboard_driver_poll(void);


// Not really a heap allocator but a pool allocator
bool init_heap_memory_allocator(void);
void* heap_malloc(int size);
void heap_free(void* pointer);

int run_unittests_stack(void);
int run_unittests_heap_allocator(void);


void append_string(unsigned char* const destination, int destination_size, const unsigned char* const source, int source_size);


/*******************************************************************************
 *                                    STUBS                                    *
 *******************************************************************************/

// Stubs for parts of code developed in userspace and then copied here

void* malloc(int size)
{
    return heap_malloc(size);
}

void free(void* pointer)
{
    heap_free(pointer);
}


// These functions are not available here
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

// TODO: Make proper include
void parse_BIOS_Data_Area(void);

terminal_contex_t shell_terminal;
terminal_contex_t log_terminal;

// TODO: Move this function to util.c and make a proper header
void long_to_hex(long int number, char * string_buffer, int string_buffer_len, unsigned char base);

// TODO: Move this function to util.c
int strlen_logless(const char * input_string)
{
    if (input_string == NULL) return 0;

    int i = 0;

    //for(i = 0; input_string[i] != 0; i++)
    while (input_string[i] != 0)
    {
        i++;

        if (i > 100)
        {
            //LOG("ERROR: strlen() smashed protection limit!");
            return 0;
        }
    }

    return i;
}

void LOG(const unsigned char* const message)
{
    int timestamp = get_timestamp();

    char str_with_timestamp[TERMINAL_MAX_X] = "AbcdAbcd: ";

    long_to_hex(timestamp, str_with_timestamp, 8, 16);

    int message_size = strlen_logless(message);

    // TODO: Add terminal_print() (without new line) so we can skip appending
    append_string(str_with_timestamp, sizeof(str_with_timestamp), message, message_size+1); // TODO: +1 because append didn't null-terminate. Fix it

    terminal_printline(&log_terminal, str_with_timestamp);
}

void stdandard_println(const unsigned char* const message) // TODO: tipfeler stdandard_println -> standard_println
{
    terminal_printline(&shell_terminal, message);
}

void check_types(void)
{
    if (sizeof(u8) != 1) goto error;
    if (sizeof(s8) != 1) goto error;
    if (sizeof(u16) != 2) goto error;
    if (sizeof(s16) != 2) goto error;
    if (sizeof(u32) != 4) goto error;
    if (sizeof(s32) != 4) goto error;

    return;

error:
    {
        unsigned char error_msg[] = "!!! ERROR: Wrong datatype size !!!";
        print_string_to_VGA_display_buffer(400, error_msg, sizeof(error_msg)-1);
        while(1);
    }
}

int a = 0;

void kernel_c_main( void )
{
    check_types();

    a = test_func(2,2,2);

    write_byte_to_IO_port(0x3F8, 'T');
    write_byte_to_IO_port(0x3F8, 'e');
    write_byte_to_IO_port(0x3F8, 's');
    write_byte_to_IO_port(0x3F8, 't');
    write_byte_to_IO_port(0x3F8, '\n');
    write_byte_to_IO_port(0x3F8, '\r');
    write_byte_to_IO_port(0x3F8, 'T');
    write_byte_to_IO_port(0x3F8, 'e');
    write_byte_to_IO_port(0x3F8, 's');
    write_byte_to_IO_port(0x3F8, 't');
    write_byte_to_IO_port(0x3F8, '\n');
    write_byte_to_IO_port(0x3F8, '\r');

    unsigned char hello_msg[] = "Hello from C code!";
    print_string_to_VGA_display_buffer(400, hello_msg, sizeof(hello_msg)-1);

    //unsigned char shell_header[] = "-----------------------------------/ Shell /-----------------------------------";
    unsigned char shell_header[] = "-----------------------------------/ SHELL /-----------------------------------";
    print_string_to_VGA_display_buffer(560, shell_header, sizeof(shell_header)-1);

    //unsigned char klog_header[] = "--------------------------------/ Kernel log /---------------------------------";
    unsigned char klog_header[] = "--------------------------------/ KERNEL LOG /---------------------------------";
    print_string_to_VGA_display_buffer(1360, klog_header, sizeof(klog_header)-1);


    // New terminal
    terminal_init(&shell_terminal,  8, 8, ENABLE_INPUT_LINE);
    terminal_init(&log_terminal,   18, 5, DISABLE_INPUT_LINE);


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

    // Re-init memory allocator after tests
    init_heap_memory_allocator();

    parse_BIOS_Data_Area();

    while(1)
    {
        keyboard_driver_poll();



        // TODO: First we need to program PIC otherwise the CPU will never be awaken
        //halt_cpu();
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
        LOG("ERROR: heap_malloc() failed - size argument too big!");
        return NULL; // QTODO: Log an error
    }

    unsigned char index = 0;
    void* calculated_address = NULL;

    if (stack_pop(heap_bin.free_blocks_stack, &index))
    {
        if (index >= heap_bin.block_count)
        {
            LOG("ERROR: heap_malloc() failed - invalid bin index!");
        }
        else
        {
            unsigned int offset = index * (heap_bin.block_size + BIN_RED_ZONE_SIZE);

            calculated_address = (void*)(heap_bin.buffer_start + offset);

            LOG("INFO: heap_malloc() - memory successfully allocated");
        }
    }
    else LOG("ERROR: heap_malloc() failed - No free memory!");

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
        LOG("ERROR: heap_free() failed - wrong index?");
    }
    else
    {
        LOG("INFO: heap_free() - memory successfully freed");
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
 *                                     UTIL                                    *
 *******************************************************************************/

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


// TODO: Move this to Util.c
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

// TODO: Move this to Util.c
// TODO: Add explicit null-terminator
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


/*******************************************************************************
 *                            Integration callbacks                            *
 *******************************************************************************/

// TODO: Move this into callbacks/integration file

void event_on_keypress(u8 key)
{
    terminal_on_keypress(&shell_terminal, key);

#if 0
    char string[] = "Key is pressed:  ";
    string[sizeof(string) -2] = key;
    terminal_printline(&shell_terminal, string);
    //LOG(string);
#endif
}
