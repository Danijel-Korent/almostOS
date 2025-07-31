
unsigned char* const UART = (unsigned char*) 0x10000000;

#define LSR_DATA_READY 0x01

static void tty_write(char data)
{
    *UART = data;
}

static char tty_read(void)
{
    unsigned char line_status_reg = *(UART + 5);

    if ((line_status_reg & LSR_DATA_READY) == 0)
    {
        return 0xff;
    }

    return *UART;
}

static void print_string(const char *str)
{
    while (*str != '\0')
    {
        tty_write(*str);
        str++;
    }
}


void c_test_main(void)
{
    unsigned const char *hello_msg = "Hello from C !\n";

#if 0
    while (*hello_msg != '\0')
    {
        *UART = *hello_msg;
        hello_msg++;
    }
#endif

    print_string(hello_msg);


    //*UART = 'A';
    //*UART = 'B';
    //*UART = 'C';

    while (1)
    {
        char val = tty_read();

        if (val != 0xff)
        {
            tty_write(val);
        }
    }
}
