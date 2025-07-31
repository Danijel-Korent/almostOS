#ifndef _KERNEL_STDDEF_H_
#define _KERNEL_STDDEF_H_

// TODO: Replace this with "system_headers"

typedef signed   char s8;
typedef unsigned char u8;

typedef signed   short s16;
typedef unsigned short u16;

typedef signed int   s32;
typedef unsigned int u32;

typedef signed long long   s64;
typedef unsigned long long u64;

#define NULL ((void*)0)

typedef unsigned int bool;
#define false (0) // Only zero is "false" in C, everything else is "true"
#define true  (1)

#endif // _KERNEL_STDDEF_H_