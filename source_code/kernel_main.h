#ifndef _KERNEL_MAIN_H_
#define _KERNEL_MAIN_H_

#include "kernel_stddef.h"

void LOG(const unsigned char* const message);
void stdandard_println(const unsigned char* const message);

void* malloc (int   size);
void  free   (void* pointer);


#endif // _KERNEL_MAIN_H_
