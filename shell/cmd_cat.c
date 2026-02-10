/**
 * @file cmd_cat.c
 *
 * @brief Implements the basic behavior of a popular Unix command "cat"
 */

#include "cmd_cat.h"

//#include "util.h"
//#include "string.h"
#include "kernel_stdio.h"
//#include "kernel_stddef.h"

#include "filesystem_router.h"

#include "../fs/tinyfs/code/rootdir_cluster.h" // TODO: TEMP - This needs to be handled better
#include "../fs/tinyfs/code/cluster_operations.h" // TODO: TEMP - This needs to be handled better


void command_cat(const char* filename)
{
#if 0
    //if (ERROR(filename == NULL)) return;
    if (filename == NULL) return;

    // # 1 Find file entry
    int32_t index = rootdir_find_file(filename);

    if (index < 0) {
        kernel_println("cat: File not found!");
        return;
    }

    // # 2) Get data cluster number and size
    struct file_entry entry;

    if (rootdir_get_file_entry(index, &entry) != 0)
    {
        return;
    }

    // # 3) Fetch all cluster data
    for (int i = 0; i<entry.file_size; i++) {

        uint8_t byte;

        cluster_read_8bit(entry.data_cluster, i, &byte);

        if (byte >= 0)
        {
            if (byte < 32) byte = '.';

            kernel_putchar(byte);
        }
    }

#else
    char buffer[1024];
    int ret_len = read_file(filename, buffer, sizeof buffer);

    buffer[sizeof buffer -1] = 0;

    for (int i = 0; i < ret_len; i++)
    {
        uint8_t byte = buffer[i];

        if (byte < 32) byte = '.';

        kernel_putchar(byte);
    }
#endif

    kernel_println("");
}


void execute__cat(int argc, char* argv[])
{
    if (argc > 1)
    {
        command_cat(argv[1]);
    }
    else
    {
        kernel_println("cat: Missing arguments!");
    }
}

