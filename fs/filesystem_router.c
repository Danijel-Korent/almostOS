#include "filesystem_router.h"

#include "kernel_stdio.h"
#include "util.h"
#include "string.h"
#include "system_headers/string.h" // TODO: I need to do something about util/string.h and system_headers/string.h

// TinyFS funcs
#include "fs_operations.h"
#include "tinyfs/images/image__cluster_size_100.h"

#include "../fs/tinyfs/code/rootdir_cluster.h" // for struct file_entry;  TODO: TEMP - This needs to be handled in the fs_operations.c/h acting as frontend
#include "../fs/tinyfs/code/cluster_operations.h" // for cluster_read_8bit();  TODO: TEMP - This needs to be handled better


void fs_router_init(void)
{
    // TODO: For now we just assume TinyFS will always be used
    if (fs_load_ramdisk(DISK_IMAGE) == 0)
    {
        kernel_println("\nLoading initial RAM disk: SUCCESS");
    }
    else
    {
        // TODO: Should probably panic if this fails
        kernel_println("\nLoading initial RAM disk: FAILED");
    }

    kernel_println("");
}

struct mount_table_entry
{
    int fs_type;
    char directory_path[64];
};

struct  mount_table_entry mount_table[8] = {};

// TODO: At one point I will have to add source device or some kind of a handler for the source of FS,
//       but for now every FS type will be hardcoded to a single source instance
//       eventualy it will need to support this arguments: https://man7.org/linux/man-pages/man2/mount.2.html
int mount_fs(int fs_type, const char* directory_path)
{
    if (fs_type == FS_TYPE__NONE || directory_path == NULL)
    {
        kernel_println("get_first_dir_in_path(): Invalid args!");
        return -1;
    }

    int mount_list_count = sizeof(mount_table) / sizeof(mount_table[0]);

    // TODO: CHeck if directory_path fits in sizeof mount_table[i].directory_path
    // TODO: Check if directory_path is not already used and reject if it does

    for(int i = 0; i < mount_list_count; i++)
    {
        if (mount_table[i].fs_type == FS_TYPE__NONE)
        {
            mount_table[i].fs_type = fs_type;
            mem_copy(mount_table[i].directory_path, sizeof mount_table[i].directory_path, directory_path, strlen_unsafe(directory_path));

            // Null-terminate, just in case
            mount_table[i].directory_path[sizeof(mount_table[i].directory_path) -1] = 0;

            // TODO TEMP: Print the table content
            kernel_printf("\nMOUNT TABLE: \n");
            for(int x = 0; x < mount_list_count; x++)
            {
                kernel_printf("[%d] Type = %d, Dir = %s\n", x, mount_table[x].fs_type, mount_table[x].directory_path);
                //kernel_printf("mount_list_count = %d\n", mount_list_count);
            }

            return i;
        }
    }

    kernel_printf("mount_fs(): some kind of error happened, I guess \n");
    return -1; // TODO: I need to define error values
}

int get_first_dir_in_path(const char *dir_path, char *output_buf, int output_buf_len)
{
    int path_len = strlen_unsafe(dir_path);

    if (path_len < 2)
    {
        kernel_println("get_first_dir_in_path(): dir_path too small!");
        return -1; // TODO: ret error
    }

#if 0
    kernel_printf("Called get_first_dir_in_path(): \n");
    kernel_printf("  dir_path       = %s \n", dir_path);
    kernel_printf("  dir_path_len   = %d \n", path_len);
    kernel_printf("  output_buf     = %x \n", output_buf);
    kernel_printf("  output_buf_len = %d \n", output_buf_len);
#endif

    if (dir_path == NULL || output_buf == NULL || output_buf_len == 0)
    {
        kernel_println("get_first_dir_in_path(): Invalid args!");
        return -1;
    }

    // Skip the root dir if present
    if (dir_path[0] == '/')
    {
        dir_path++;
        path_len--;
    }

    for(int i = 0; (dir_path[i] != '/') && (i < path_len); i++)
    {
        //kernel_printf("get_first_dir_in_path(): iter #%d! \n", i);
        *output_buf = dir_path[i];
        output_buf++;
        //kernel_printf("get_first_dir_in_path(): output_buf = %s \n", output_buf);

        output_buf_len--;
        if (output_buf_len == 0)
        {
            kernel_printf("get_first_dir_in_path(): Output buffer too small! \n");
            return -1; // TODO: Log error
        }
    }

    *output_buf = 0; // Null-terminate
    //kernel_printf("get_first_dir_in_path(): returning = %s \n", output_buf);
    return 0;
}

static int find_mount(const char *dir_path, char *mount_path /* out */, int mount_path_size, int *fs_type /* out */)
{
    if (dir_path == NULL || mount_path == NULL || fs_type == NULL)
    {
        kernel_println("find_mount(): Invalid args!");
        return -1;
    }

#if 0
    kernel_printf("Called find_mount(): \n");
    kernel_printf("  dir_path   = %s \n", dir_path);
    kernel_printf("  fs_type         = %x \n", fs_type);
    kernel_printf("  mount_path      = %x \n", mount_path);
    kernel_printf("  mount_path_size = %d \n", mount_path_size);
#endif

    int mount_list_count = sizeof(mount_table) / sizeof(mount_table[0]);

    for(int i = 0; i < mount_list_count; i++)
    {
        if (mount_table[i].fs_type != FS_TYPE__NONE)
        {
            // TODO 

            char input_dir_name[32] = {0};
            char mount_dir_name[32] = {0};

            if (get_first_dir_in_path(dir_path, input_dir_name, sizeof input_dir_name) < -1)
            {
                kernel_println("find_mount(): error in get_first_dir_in_path(dir_path)");
                return -1;
            }



            if (get_first_dir_in_path(mount_table[i].directory_path, mount_dir_name, sizeof mount_dir_name) < -1)
            {
                kernel_println("find_mount(): error in get_first_dir_in_path(mount_table[i].directory_path)");
                return -1;
            }

#if 0
            kernel_printf("\n  input_dir_name   = %s \n", input_dir_name);
            kernel_printf(  "  mount_dir_name   = %s \n", mount_dir_name);
#endif
            if(strncmp(input_dir_name, mount_dir_name, sizeof input_dir_name) == 0)
            {
                // TODO: !!! This copying doesn't make any sense. I should just return "i" and use it to get the data from the table !!!
                mem_copy(mount_path, mount_path_size, mount_table[i].directory_path, sizeof mount_table[i].directory_path);
                *fs_type = mount_table[i].fs_type;
                //kernel_printf(" !!!! USED strncmp !!!!!");
                return i;
            }
        }
    }

    kernel_println("find_mount(): Mount not found!");
    return -1;
}

// For now lets just return a stream of null-terminated strings (in buffer) and see where this will take me
// Returns the size of filled buffer. Maybe I could use AI to generate documentation?
int get_list_of_files(const char* directory_path, char* buffer, int buffer_len)
{
    //kernel_printf("[get_list_of_files(): directory_path = %s] \n", directory_path);

#if 0
    static const char test_ret[] = "file_1\0" "file_2\0" "file_3\0";

    //kernel_printf("[get_list_of_files] buffer_len = %d \n", buffer_len);
    //kernel_printf("[get_list_of_files] sizeof test_ret = %d \n", sizeof test_ret);

    return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
#endif

    // TODO: Dirty hack until I finish everything else and see how to deal with this
    //       I'm pretty sure that in Linux that the folder used as mounting point needs to exist before calling mount (but I need to check to be 100% sure)
    //       But at the moment TinyFS doesn't support folders so I cannot use it for this by making it root fs. Will probably add some kind of virtual fs
    if (directory_path[0] == '/' && directory_path[1] == 0)
    {
        int mount_list_count = sizeof(mount_table) / sizeof(mount_table[0]);

        // TODO: CHeck if directory_path fits in sizeof mount_table[i].directory_path
        // TODO: Check if directory_path is not already used and reject if it does
        int output_len = 0;

        for(int i = 0; i < mount_list_count; i++)
        {
            if (mount_table[i].fs_type != FS_TYPE__NONE)
            {

                //mount_table[i].directory_path

                int len = mem_copy(buffer, buffer_len-1, mount_table[i].directory_path, strlen_unsafe(mount_table[i].directory_path));

                buffer += len;
                *buffer = 0;
                buffer++;
                len++; // Add space for null pointer

                buffer_len -= len;
                output_len += len;
                // TODO TEMP: Print the table content
                //kernel_printf("\nMOUNT TABLE: \n");
                //for(int x = 0; x < mount_list_count; x++)
                {
                //    kernel_printf("[%d] Type = %d, Dir = %s\n", x, mount_table[x].fs_type, mount_table[x].directory_path);
                    //kernel_printf("mount_list_count = %d\n", mount_list_count);
                }
            }
        }
        return output_len;
    }

    int fs_type = FS_TYPE__NONE;
    char mount_path[64] = {0}; // Don't care about the perf

    if (find_mount(directory_path, mount_path, sizeof mount_path, &fs_type) < 0)
    {
        kernel_println("get_list_of_files(): failed in find_mount");
        return -1;
    }

    // TODO: For initial impl it is hardcoded if/else switch. Later it will be replaced with func pointer interface, I swear!
    if (fs_type == FS_TYPE__TINY_FS)
    {
        // TODO: since TinyFS doesn't support folders at the moment, this is only useful in read_file()
        //const char* fs_relative_path = directory_path;
        // increase fs_relative_path up to the point where directory_path[i] and directory_path[i] start being different
        // after increase, if the directory_path[0] == '/', increase once more


        int32_t count = rootdir_get_table_size();

        struct file_entry entry;

        int output_len = 0;

        char *orig_buffer = buffer;

        for (int i = 0; i < count; i++)
        {
            if (rootdir_get_file_entry(i, &entry) == 0)
            {
#if 0
                kernel_printf("\n[entry.name         = %s] \n", entry.name);
                kernel_printf(  "[strlen(entry.name) = %d] \n", strlen_unsafe(entry.name));
#endif
                int len = mem_copy(buffer, buffer_len-1, entry.name, strlen_unsafe(entry.name));

                buffer += len;
                *buffer = 0;
                buffer++;
                len++; // Add space for null pointer

                buffer_len -= len;
                output_len += len;

                //kernel_printf("[orig_buffer = %s] \n", orig_buffer);
            }
        }

        return output_len;
    }
    else if (fs_type == FS_TYPE__BLOB_FS)
    {
        // TODO: To implement
        kernel_println("get_list_of_files(): BlobFS not yet integrated!");
        return -1;
    }
    else if (fs_type == FS_TYPE__TEST_FS)
    {
        static const char test_ret[] = "file1\0" "file2\0" "file3\0";

        //kernel_printf("[get_list_of_files] buffer_len = %d \n", buffer_len);
        //kernel_printf("[get_list_of_files] sizeof test_ret = %d \n", sizeof test_ret);

        return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
    }
}

struct file_info_entry
{
    uint32_t size;
    uint8_t is_directory;
};

int get_file_info(const char *file_path, struct file_info_entry *file_info)
{
    // TODO: This function is unimplemented !!!
    if (file_path == NULL || file_info == NULL) return -1;

    // For testing always say it is a directory
    file_info->is_directory = 1;
}

// 1st version not using handler to avoid having to track any state
// The syscall should in the end look like this: https://man7.org/linux/man-pages/man2/read.2.html
int read_file(const char* file_path, char* buffer, int buffer_len)
{
    //kernel_printf("[read_file(): file_path = %s] \n", file_path);

    int fs_type = FS_TYPE__NONE;
    char mount_path[64] = {0}; // Don't care about the perf

    if (find_mount(file_path, mount_path, sizeof mount_path, &fs_type) < 0)
    {
        kernel_println("read_file(): failed in find_mount");
        return -1;
    }

    //kernel_printf("[read_file(): mount found = %s] \n", mount_path);

    file_path += strlen_unsafe(mount_path) +1;

    //kernel_printf("read_file(): mount relative file path: %s \n\n", file_path);

    // For initial impl it is hardcoded if/else switch. Later it will be replaced with func pointer interface, I swear!
    if (fs_type == FS_TYPE__TINY_FS)
    {
        // TODO: since TinyFS doesn't support folders at the moment, this is only useful in read_file()
        //const char* fs_relative_path = directory_path;
        // increase fs_relative_path up to the point where directory_path[i] and directory_path[i] start being different
        // after increase, if the directory_path[0] == '/', increase once more

        //if (ERROR(filename == NULL)) return;
        if (file_path == NULL) return -1;

        // # 1 Find file entry
        int32_t index = rootdir_find_file(file_path);

        if (index < 0) {
            kernel_println("read_file: File not found!");
            return -1;
        }

        // # 2) Get data cluster number and size
        struct file_entry entry;

        if (rootdir_get_file_entry(index, &entry) != 0)
        {
            return -1;
        }

        int output_len = 0;

        // # 3) Fetch all cluster data
        for (int i = 0; i<entry.file_size; i++) {

            uint8_t byte;

            cluster_read_8bit(entry.data_cluster, i, &byte);

            *buffer++ = byte;
            buffer_len--;
            output_len++;

            if (byte >= 0)
            {
                //if (byte < 32) byte = '.';

                //kernel_putchar(byte);
            }
        }

        //kernel_printf("\n\n\n");

        return output_len;
    }
    else if (fs_type == FS_TYPE__BLOB_FS)
    {
        // TODO: To implement
        kernel_println("get_list_of_files(): BlobFS not yet integrated!");
        return -1;
    }
    else if (fs_type == FS_TYPE__TEST_FS)
    {
        // If I put string literal directly into strncmp() the compiler will replace it with strcmp()
        // I don't have strcmp implemented and am in no mood to implement it now
        const char *file1_path = "file1";
        const char *file2_path = "file2";
        const char *file3_path = "file3";

        if(strncmp(file_path, file1_path, 30) == 0)
        {
            static const char test_ret[] = "Hardcoded file1";
            return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
        }

        if (strncmp(file_path, file2_path, 30) == 0)
        {
            static const char test_ret[] = "Hardcoded file2";
            return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
        }

        if (strncmp(file_path, file3_path, 30) == 0)
        {
            static const char test_ret[] = "Hardcoded file3";
            return mem_copy(buffer, buffer_len, test_ret, sizeof test_ret);
        }

        //kernel_printf("[get_list_of_files] buffer_len = %d \n", buffer_len);
        //kernel_printf("[get_list_of_files] sizeof test_ret = %d \n", sizeof test_ret);

        return -1;
    }
}
