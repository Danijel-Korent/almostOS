#include "disk_image.h"
#include "fs_operations.h"
#include "rootdir_cluster.h"
#include "cluster_operations.h"
#include "util.h"

int fs_load_ramdisk(uint8_t* const ramdisk_image)
{
    return load_ramdisk(ramdisk_image);
}

int fs_load_disk_image(const char* const image_filename)
{
    TRACE("Called with image_filename: %s", image_filename);

    int ret = -1;

    ret = load_disk(image_filename);

    if (ret != 0) {
        ret = create_disk();
    }

    return ret;
}

int fs_store_disk_image(const char* const image_filename)
{
    TRACE("Called with image_filename: %s", image_filename);

    int ret = store_disk(image_filename);

    // free_disk();

    return ret;
}

int fs_create_file(const char* const filename)
{
    return rootdir_create_file(filename);
}

int fs_delete(const char* const filename)
{
    TRACE("Called with filename: %s", filename);

    int32_t index = rootdir_find_file(filename);

    if (index >= 0) {
        return rootdir_delete_file(index);
    }

    return index;
}

int fs_open(const char* const filename)
{
    TRACE("Called with filename: %s", filename);
    // TODO: Check if filename has only allowed characters

    return rootdir_find_file(filename);
}

int fs_write(int handler, const uint8_t* buffer_ptr, uint32_t buffer_len)
{
    TRACE("Called with handler %i, ptr: %p, len: %i", handler, buffer_ptr, buffer_len);

    if (handler < 0) {
        TRACE("Bad handler: %i", handler);
        return handler;
    }

    struct file_entry file;

    if (rootdir_get_file_entry(handler, &file) != 0){
        TRACE("Unable to fetch file entry for index: %i", handler);
        return -1;
    }

    int ret = cluster_write_data(file.data_cluster, file.file_size, buffer_ptr, buffer_len);

    if (ret == RET_SUCCESS) {
        ret = rootdir_set_file_size(handler, file.file_size + buffer_len);
    }

    return ret;
}

int fs_close(int handler)
{
    // Nothing to do here since I don't allocate handler or maintain any state on opened files
    return RET_SUCCESS;
}
