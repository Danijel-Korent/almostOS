
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

#include "kernel_main.h"

#include "kernel_stddef.h"
#include "terminal.h"
#include "instruction_wrappers.h"
#include "malloc.h"

#include "util.h"
#include "string.h"
#include "BIOS_Data_Area.h"
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

int run_unittests_stack(void);


/*******************************************************************************
 *                                    STUBS                                    *
 *******************************************************************************/

// Stubs for parts of code developed in userspace and then copied here

void* malloc(u32 size)
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

terminal_contex_t shell_terminal;
terminal_contex_t log_terminal;

void LOG(const unsigned char* const message)
{
    int timestamp = get_timestamp();

    char str_with_timestamp[TERMINAL_MAX_X] = "AbcdAbcd: ";

    long_to_hex(timestamp, str_with_timestamp, 8, 16);

    int message_size = strlen_unsafe_logless(message);

    // TODO: Add terminal_print() (without new line) so we can skip appending
    append_string(str_with_timestamp, sizeof(str_with_timestamp), message, message_size+1); // TODO: +1 because append didn't null-terminate. Fix it

    terminal_printline(&log_terminal, str_with_timestamp);
}

void kernel_println(const unsigned char* const message)
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
