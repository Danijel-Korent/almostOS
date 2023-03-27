#ifndef _MALLOC_H_
#define _MALLOC_H_

#include "kernel_stddef.h"

// Not really a heap allocator but a pool allocator
bool init_heap_memory_allocator(void);
void* heap_malloc(u32 size);
void heap_free(void* pointer);

int run_unittests_heap_allocator(void);

#endif // _MALLOC_H_
