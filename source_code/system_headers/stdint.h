
// This is usually provided by compiler, but I don't know how to make compiler to
// provide only stdint.h without providing the rest of system headers

#ifndef _STDINT_H_
#define _STDINT_H_

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef short               int16_t;
typedef int                 int32_t;
typedef long long           int64_t;

#endif // _STDINT_H_
