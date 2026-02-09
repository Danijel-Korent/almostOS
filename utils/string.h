#ifndef _UTILS_STRING_H_
#define _UTILS_STRING_H_

// TODO: BIG PROBLEM -> I have two string.h files. Merge them

#include "kernel_stddef.h"

u32 strlen_unsafe(const u8* input_string);
u32 strlen_unsafe_logless(const u8* input_string);
u32 get_string_size(const u8* const buffer_ptr, u32 buffer_size);

u8* trim_string(u8* input_string);
void append_string(u8* const destination, u32 destination_size, const u8* const source, u32 source_size);


#endif
