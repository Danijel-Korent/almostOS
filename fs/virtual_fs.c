#include "virtual_fs.h"

#include "util.h"
#include "string.h"
#include "system_headers/string.h" // TODO: I need to do something about util/string.h and system_headers/string.h


int vfs_get_list_of_files(const char* directory_path, char* buffer, int buffer_len)
{
    static const char test_ret[] = "file1\0" "file2\0" "file3\0";

    //kernel_printf("[get_list_of_files] buffer_len = %d \n", buffer_len);
    //kernel_printf("[get_list_of_files] sizeof test_ret = %d \n", sizeof test_ret);

    return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
}

int vfs_read_file(const char* file_path, char* buffer, int buffer_len)
{
    // If I put string literal directly into strncmp() the compiler will replace it with strcmp()
    // I don't have strcmp implemented and am in no mood to implement it now
    const char *file1_path = "file1";
    const char *file2_path = "file2";
    const char *file3_path = "file3";

    if(strncmp(file_path, file1_path, 30) == 0)
    {
        static const char test_ret[] = "VFS Hardcoded file1";
        return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
    }

    if (strncmp(file_path, file2_path, 30) == 0)
    {
        static const char test_ret[] = "VFS Hardcoded file2";
        return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
    }

    if (strncmp(file_path, file3_path, 30) == 0)
    {
        static const char test_ret[] = "VFS Hardcoded file3";
        return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
    }

    //kernel_printf("[get_list_of_files] buffer_len = %d \n", buffer_len);
    //kernel_printf("[get_list_of_files] sizeof test_ret = %d \n", sizeof test_ret);

    return -1;
}

