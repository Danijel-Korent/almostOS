// TODO: This is not included by compiler... WHY???

#ifndef _STRING_H_
#define _STRING_H_

#include <stdint.h>

// TODO: I should probably move folder "system_headers" to "stdlib_c/headers"

uint32_t strlen(const char *str);
int strncmp(const char *str1, const char *str2, uint32_t n);

#endif // _STRING_H_
