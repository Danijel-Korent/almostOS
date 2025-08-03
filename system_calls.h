#ifndef _SYSTEM_CALLS_H_
#define _SYSTEM_CALLS_H_

#include "system_headers/stddef.h" // TODO: Better to use include without system path as custom system path confuses C InteliSense

ssize_t sys_write(int fd, const void *buf, size_t count);

#endif
