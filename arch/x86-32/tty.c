#include "tty.h"
#include "COM_port.h"

void tty_write(char data)
{
    COM_port_TX(data);
}

char tty_read(void)
{
    return COM_port_RX();
}
