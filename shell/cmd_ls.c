/**
 * @file cmd_ls.c
 *
 * @brief Implements the basic behavior of a popular Unix command "ls"
 */

#include "cmd_ls.h"

#include <stdint.h>
#include "util.h"
#include "kernel_stdio.h"

#include "filesystem_router.h"

#include "../fs/tinyfs/code/rootdir_cluster.h" // TODO: TEMP - This needs to be handled better

//#include "string.h"
//#include "kernel_stddef.h"


void execute__ls(int argc, char* argv[])
{
#if 0
    int32_t count = rootdir_get_table_size();

    struct file_entry entry;

    kernel_println("");
    //kernel_println("   IDX   SIZE   NAME.........   CLUSTERS");
    kernel_println("   IDX   SIZE   NAME");
    kernel_println("");

    for (int i = 0; i < count; i++)
    {
        if (rootdir_get_file_entry(i, &entry) == 0)
        {
            //printf("   %3i   %4i   %13s   ", i, entry.file_size, entry.name);

            // TODO: Man... I really need to implement printf()
            char line[] = "   IDX   SIZE   NAME.........   CLUSTERS"; // "CLUSTERS" is not outputed because entry.name is null-terminated
            long_to_hex(i, line + 3, 3, 10);
            long_to_hex(entry.file_size, line + 9, 4, 10);
            mem_copy(line + 16, 13, entry.name, 13);

            //print_cluster_chain(entry.data_cluster);
            kernel_println(line);
        }
    }
#else

    char *directory = "/";

    if (argc > 1) directory = argv[1];

    char buffer[2048];
    int ret_len = get_list_of_files(directory, buffer, sizeof buffer);

    //kernel_printf("[ls] ret_len = %d \n", ret_len);

    for (int i = 0; i < ret_len; i++)
    {
        if (buffer[i] == 0)
        {
            kernel_putchar('\n');
        }
        else
        {
            kernel_putchar(buffer[i]);
        }
    }
#endif

    kernel_println("");
}
