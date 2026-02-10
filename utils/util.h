#ifndef _UTIL_H_
#define _UTIL_H_

#include "kernel_stddef.h"

int memory_is_equal(const u8* const mem1, int mem1_size, u8* const mem2, int mem2_size);
int mem_copy(u8* const destination, int destination_size, const u8* const source, int source_size);
void* memset(void* ptr, int value, unsigned int num);

char byte_to_hexchar(unsigned char byte);
long int hex_to_long(const char* str);
void long_to_hex(long int number, char * string_buffer, int string_buffer_len, unsigned char base);

int atoi(const char* str);

#endif // _UTIL_H_
