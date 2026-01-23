/*******************************************************************************
 *                            Binned heap allocator                            *
 *  *actually currently just a pool memory allocator, but we will get there :3 *
 *******************************************************************************/

#include "malloc.h"

#include "stack.h"
#include "kernel_stdio.h"

struct heap_bin_contex
{
    u32 block_size;
    u32 block_count;
    u32 buffer_size;
    u8* buffer_start;
    stack_handle_t* free_blocks_stack;
};

static struct heap_bin_contex heap_bin;

#define BIN_BLOCK_COUNT    (3)      // Why only 3 blocs? Was this only used for testing??
#define BIN_BLOCK_SIZE     (128)
#define BIN_RED_ZONE_SIZE  (32)


bool init_heap_memory_allocator(void)
{
    // Setup the buffer for heap storage
    {
        static u8 heap_buffer[BIN_BLOCK_COUNT*(BIN_BLOCK_SIZE+BIN_RED_ZONE_SIZE)];

        heap_bin.block_count = BIN_BLOCK_COUNT;
        heap_bin.block_size  = BIN_BLOCK_SIZE;

        heap_bin.buffer_start = heap_buffer;
        heap_bin.buffer_size  = sizeof(heap_buffer);

        if (heap_bin.block_count * (heap_bin.block_size + BIN_RED_ZONE_SIZE) > heap_bin.buffer_size)
        {
            return false;
        }
    }

    // Setup the stack for free blocks
    {
        static stack_handle_t free_blocks_stack;
        static u8 stack_buffer[BIN_BLOCK_COUNT];

        if ( ! stack_init(&free_blocks_stack, stack_buffer, sizeof(stack_buffer)))
        {
            return false;
        }

        // Populate free blocks list
        for (int i = 0; i < BIN_BLOCK_COUNT; i++)
        {
            if ( ! stack_push(&free_blocks_stack, i))
            {
                return false;
            }
        }

        heap_bin.free_blocks_stack = &free_blocks_stack;
    }

    return true;
}

void* heap_malloc(u32 size)
{
    if (size == 0) return NULL; // QTODO: Log an error

    if (size > heap_bin.block_size)
    {
        LOG("ERROR: heap_malloc() failed - size argument too big!");
        return NULL; // QTODO: Log an error
    }

    u8 index = 0;
    void* calculated_address = NULL;

    if (stack_pop(heap_bin.free_blocks_stack, &index))
    {
        if (index >= heap_bin.block_count)
        {
            LOG("ERROR: heap_malloc() failed - invalid bin index!");
        }
        else
        {
            u32 offset = index * (heap_bin.block_size + BIN_RED_ZONE_SIZE);

            calculated_address = (void*)(heap_bin.buffer_start + offset);

            LOG("INFO: heap_malloc() - memory successfully allocated");
        }
    }
    else LOG("ERROR: heap_malloc() failed - No free memory!");

    return calculated_address;
}

void heap_free(void* pointer)
{
    if (pointer == NULL) return; // QTODO: Log an error

    u8 index = 0;
    u32 offset = 0;

    offset = (u8*)pointer - heap_bin.buffer_start;

    index = offset / (heap_bin.block_size + BIN_RED_ZONE_SIZE);

    if ( ! stack_push(heap_bin.free_blocks_stack, index))
    {
        LOG("ERROR: heap_free() failed - wrong index?");
    }
    else
    {
        LOG("INFO: heap_free() - memory successfully freed");
    }
}

int run_unittests_heap_allocator(void)
{
    int failed_test_counter = 0;

    if (init_heap_memory_allocator())
    {
        // Must store all allocated addresses for freeing later
        // -1 because buffer filled with 0xFF will be stored in separate var
        void* returned_addresses[BIN_BLOCK_COUNT -1] = {0};

        u8* buffer_with_pattern_0xff = NULL;

        // For this implementation this should not corrupt allocator
        heap_free(NULL);

        // Test for size limitation
        if (heap_malloc(BIN_BLOCK_SIZE + 1) != NULL)
        {
            failed_test_counter++;
        }

        // Allocate first block and fill it with 0xFF pattern
        // TODO: move this allocation between two other allocations
        {
            buffer_with_pattern_0xff = heap_malloc(BIN_BLOCK_SIZE);

            if (buffer_with_pattern_0xff == NULL)
            {
                failed_test_counter++;
            }

            for (int i = 0; i < BIN_BLOCK_SIZE; i++)
            {
                buffer_with_pattern_0xff[i] = 0xff;
            }
        }

        // Allocate all the rest of available blocks and color them with 0x11
        {
            for (int x = 0; x < BIN_BLOCK_COUNT -1; x++)
            {
                u8* buffer = heap_malloc(BIN_BLOCK_SIZE);

                if (buffer != NULL)
                {
                    for (int i = 0; i < BIN_BLOCK_SIZE; i++)
                    {
                        buffer[i] = 0x11;
                    }
                }
                else
                {
                    failed_test_counter++;
                }

                returned_addresses[x] = buffer;
            }
        }

        // All blocks are allocated and this malloc should fail
        {
            u8* buffer = heap_malloc(BIN_BLOCK_SIZE);

            if (buffer != NULL)
            {
                failed_test_counter++;
            }
        }

        // Free one block and they try new allocation and fill the memory
        {
            void* pointer = returned_addresses[0];

            heap_free(pointer);

            u8* buffer = heap_malloc(5);

            if (buffer == NULL)
            {
                failed_test_counter++;
            }

            returned_addresses[0] = buffer;
        }

        // Free all memory blocks except the one with 0xFF pattern
        {
            for (int i = 0; i<sizeof(returned_addresses); i++)
            {
                heap_free(returned_addresses[i]);
            }
        }

        // Check that the first allocation still holds the 0xFF pattern
        {
            for (int i = 0; i < BIN_BLOCK_SIZE; i++)
            {
                if (buffer_with_pattern_0xff[i] != 0xff)
                {
                    failed_test_counter++;
                }
            }

            heap_free(buffer_with_pattern_0xff);
        }
    }
    else
    {
        failed_test_counter++;
    }

    return failed_test_counter;
}
