#ifndef _POORS_MAN_KEYBOARD_DRIVER_H_
#define _POORS_MAN_KEYBOARD_DRIVER_H_

#include "kernel_stddef.h"

void keyboard_driver_poll(void);

void event_on_keypress(u8 key);

#endif // _POORS_MAN_KEYBOARD_DRIVER_H_
