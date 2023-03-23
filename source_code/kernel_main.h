#ifndef _KERNEL_MAIN_H_
#define _KERNEL_MAIN_H_

#include "kernel_stddef.h"

void* malloc (u32   size);
void  free   (void* pointer);

// Just a stub func defined as empty function
int printf ( const char * format, ... );

#endif // _KERNEL_MAIN_H_
