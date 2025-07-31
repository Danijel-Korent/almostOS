/**
 * @brief Contains kernel_c_main() a main entry point into C code from ASM code. Also contains type checks and
 *        stubs for functions not yet implemented
 *
 */

// This needs to be compiled with -nostartfiles and -nostdlib since standard lib is not available

/*

    TODO NEXT:
        - Create and load IDT table so I can use interrupts/PIC
            - and I need interrupts so I can use the "hlt" instruction
            - and I need the "hlt" instructions so that the OS doesn't butcher
              the CPU, which kind of annoys the users of this wonderful, wonderful, OS

        - Enable Wall i Werror

        - Add makefile rule for installing all required APT packages
            - Also add it to description / readme

        - Makefile: set GCC system include path to this folder or just disable it

        - Implement checks for buffer overflow


    TODO LIST:
        - FEATURE: Implement basic terminal functionality
        - FEATURE: Implement logging facilities
        - FEATURE: Integrate shell and FAT driver from "FAT-filesystem-driver" repo
                    - Fat driver needs to be finished and also needs serious cleaning
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

    http://www.cs.cmu.edu/~ralf/files.html (a comprehensive listing of interrupt calls, I/O ports, memory locations, far-call interfaces)
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
#include "stack.h"
#include "kernel_stdio.h"

#include "util.h"
#include "string.h"
#include "BIOS_Data_Area.h"
#include "poors_man_VGA_driver.h"
#include "poors_man_keyboard_driver.h"
#include "arch/tty.h"

#include "fs_operations.h"
#include "fs/tinyfs/images/image__cluster_size_100.h"

#include "scheduler.h"


static void tty_input_poll(void);
static void check_types(void);

extern void* STACK_MEM_START;
extern void* STACK_MEM_END;

// Entry point for kernel C code
void kernel_c_main( void )
{
    int local_var = 0;
    check_types();

    static int a;
    a = test_func(2,2,2);


    unsigned char hello_msg[] = "Hello from C code!";
    print_string_to_VGA_display_buffer(400, hello_msg, sizeof(hello_msg)-1);

    kernel_stdio_init();

    kernel_println("\n\nStarting kernel... \n");

    kernel_printf("kernel_c_main():  entry addr      = %x \n", kernel_c_main);
    kernel_printf("kernel_c_main():  STACK_MEM_START = %x \n", &STACK_MEM_START);
    kernel_printf("kernel_c_main():  &local_var      = %x \n", &local_var);
    kernel_printf("kernel_c_main():  &hello_msg      = %x \n", &hello_msg);
    kernel_printf("kernel_c_main():  STACK_MEM_END   = %x \n\n", &STACK_MEM_END);


    // QTODO: this repeating code needs a function of its own
    if (run_unittests_stack() == 0)
    {
        kernel_println("Stack unittests: PASSED");
    }
    else
    {
        kernel_println("Stack unittests: FAILED");
    }

    if (run_unittests_heap_allocator() == 0)
    {
        kernel_println("Memory allocator unittests: PASSED");
    }
    else
    {
        kernel_println("Memory allocator unittests: FAILED");
    }

    // Re-init memory allocator after tests
    init_heap_memory_allocator();

    parse_BIOS_Data_Area();

    if (fs_load_ramdisk(DISK_IMAGE) == 0)
    {
        kernel_println("Loading initial RAM disk: SUCCESS");
    }
    else
    {
        kernel_println("Loading initial RAM disk: FAILED");
    }

    kernel_println("");

    scheduler_init();

    // TEMP until taht terminal code abomination gets removed
    event_on_keypress(13);

    while(1)
    {
        // This is where currently all the actions takes place
        // When the driver detects keypress, it call a callback for terminal input,
        // and terminal then potentially calls other commands and updates the VGA display
        keyboard_driver_poll();

        tty_input_poll();

        // Currently only cooperative scheduling is implemented, so each thread/process
        // yields CPU time voluntarily
        schedule();

        // TODO: First we need to program PIC otherwise the CPU will never be awaken
        //halt_cpu();
    }
}

void tty_input_poll(void)
{
    u8 val = tty_read();

#if 0
    {
        char code_str[15] = "AbcdAbcd";

        long_to_hex(val, code_str, 8, 16);
        //long_to_hex(val & ~0x80, code_str, 8, 16);

        kernel_println(code_str);
    }
#endif

    if (val != 0xff)
    {
        event_on_keypress(val & 0x7f);
    }
}

static void check_types(void)
{
    if (sizeof(u8) != 1) goto error;
    if (sizeof(s8) != 1) goto error;
    if (sizeof(u16) != 2) goto error;
    if (sizeof(s16) != 2) goto error;
    if (sizeof(u32) != 4) goto error;
    if (sizeof(s32) != 4) goto error;
    if (sizeof(u64) != 8) goto error;
    if (sizeof(s64) != 8) goto error;

    return;

error:
    {
        unsigned char error_msg[] = "!!! ERROR: Wrong datatype size !!!";
        print_string_to_VGA_display_buffer(400, error_msg, sizeof(error_msg)-1);
        while(1);
    }
}

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
