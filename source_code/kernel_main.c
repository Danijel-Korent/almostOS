
// This needs to be compiled with -nostdlib since there is not standard lib available

/*

    TODO NEXT:
        - Add function "event_on_keypress(key)" - a callback for hooking up a processing code
        - Create a scancode-to-ascii conversion table

    TODO LIST:
        - BUG:        Keyboard output is printed on key release instead on key press
        - TODO:    Create header for equivalent of stdint.h types
        - FEATURE: Implement polling driver for a keyboard (reading + scancode conversion)
        - FEATURE: Implement basic terminal emulator
        - FEATURE: Integrate shell and FAT driver from "FAT-filesystem-driver" repo
        - FEATURE: Implement logging facilities
        - FEATURE: Implement serial/UART driver
        - FEATURE: Implement timer support
        - FEATURE: Implement support for the CPUID

*/


void output_char(int position, unsigned char ch);
void print_string(int position, unsigned char* string, int string_size);
void keyboard_driver_poll(void);

// From startup.asm
// TODO: move both implementation and declaration into seperate files
int test_func(int base, int multiplier, int adder);
unsigned char read_byte_from_IO_port( unsigned short port_address);


void kernel_c_main( void )
{
    for (int i = 400; i < 1000; i++)
    {
        //output_char(i, 'A'); // Just to see if this function is actually executed
    }

    unsigned char hello_msg[] = "Hello from C code!";

    print_string(400, hello_msg, sizeof(hello_msg)-1);

    //int test = test_func(34, 2);

    output_char(640, test_func(34, 2, 1));
    output_char(642, test_func(35, 2, 1));
    output_char(644, test_func(36, 2, 1));


    while(1)
    {
        keyboard_driver_poll();
    }
}

void output_char(int position, unsigned char ch)
{
    static unsigned char* const VGA_RAM = 0x000B8000;

    position = position * 2; // One char takes 16 bits in VGA RAM (8bit char to display + 8bit for color)

    VGA_RAM[position]   = ch;
    VGA_RAM[position+1] = 0x13; // QTODO: hardcoded color - add arguments for foreground and background color
}

void print_string(int position, unsigned char* string, int string_size)
{
    for( int i = 0; i < string_size; i++)
    {
        output_char(position + i, string[i]);
    }
}

/*******************************************************************************
 *                               Keyboard driver                               *
 *******************************************************************************/

void keyboard_driver_poll(void)
{
    static unsigned char previous_scan_code = 0;
    static int i = 800;

    if (i > 1200) i = 800;

    unsigned char scan_code = read_byte_from_IO_port(0x60);

    scan_code += 49 - 2;

    if ((previous_scan_code != scan_code) && ((scan_code & 0x80) == 0x80))
    {
        output_char(i, scan_code & ~0x80);
        i++;
    }

    previous_scan_code = scan_code;
}


/*

 *******************************************************************************
 *                                 General IO                                  *
 *******************************************************************************

Legacy IBM PC:

    http://www.cs.cmu.edu/~ralf/files.html (famous Ralf Brown's interrupt list)
    http://opensecuritytraining.info/IntroBIOS_files/Day1_04_Advanced%20x86%20-%20BIOS%20and%20SMM%20Internals%20-%20IO.pdf


 *******************************************************************************
 *                              Memory Mapped IO                               *
 *******************************************************************************



 *******************************************************************************
 *                                 x86 IO bus                                  *
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

*/