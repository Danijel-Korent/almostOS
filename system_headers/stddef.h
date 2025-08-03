
// This is usually provided by compiler, but I don't know how to make compiler to
// provide only stdint.h without providing the rest of system headers

#ifndef _STDDEF_H_
#define _STDDEF_H_

#define NULL ((void*)0)
typedef unsigned long size_t;
typedef signed   long ssize_t;

#endif // _STDDEF_H_
