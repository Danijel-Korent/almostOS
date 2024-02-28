#define FUSE_USE_VERSION 31
//#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
//#include <sys/stat.h>

#include "fs_operations.h"
#include "../code/util.h"
#include "../code/rootdir_cluster.h"
#include "../code/cluster_operations.h"
#include "../code/FAT_cluster.h"


void print_cluster_chain(uint32_t head_cluster)
{
    printf("%02x", head_cluster);

    uint32_t next_cluster = FAT_get_linked_cluster(head_cluster);

    if (next_cluster == 0xFF)
    {
        return;
    }

    printf(" -> ");

    print_cluster_chain(next_cluster);
}

void print_FAT_table(void)
{
    printf("\nAllocated clusters: \n");
    for (int i = 1; i < 30; i++)
    {
        uint32_t next_cluster = FAT_get_linked_cluster(i);

        printf("Cluster ");

        if (next_cluster != 0) {
            print_cluster_chain(i);
        } else {
            printf("FREE");
        }

        printf("\n");
    }
}

void print_root_dir_table(void)
{
    int32_t count = rootdir_get_table_size();

    struct file_entry entry;

    printf("\n");
    printf("   %3s   %4s   %13s   CLUSTERS \n", "IDX", "SIZE", "NAME.........");

    for (int i = 0; i < count; i++) {
        if (rootdir_get_file_entry(i, &entry) == 0){
            printf("   %3i   %4i   %13s   ", i, entry.file_size, entry.name);
            print_cluster_chain(entry.data_cluster);
            printf("\n");
        }
    }

    printf("\n");
}

static int fuse_getattr(const char *path, struct stat *file_stat) {

    TRACE("Called with path = %s, file_stat = %p", path, file_stat);

    memset(file_stat, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        file_stat->st_mode = S_IFDIR | 0755;
        file_stat->st_nlink = 2;
        return 0;
    }

    if (strcmp(path, "/debug") == 0) {
        file_stat->st_mode = S_IFDIR | 0755;
        file_stat->st_nlink = 2;
        return 0;
    }

    if (strcmp(path, "/debug/fat_table") == 0) {
        file_stat->st_mode = S_IFREG | 0777;
        file_stat->st_nlink = 1;
        file_stat->st_size = 1024;
        return 0;
    }

    if (strcmp(path, "/debug/fat_table") == 0) {
        file_stat->st_mode = S_IFREG | 0777;
        file_stat->st_nlink = 1;
        file_stat->st_size = 4*1024;
        return 0;
    }

    if (strcmp(path, "/debug/root_dir_table") == 0) {
        file_stat->st_mode = S_IFREG | 0777;
        file_stat->st_nlink = 1;
        file_stat->st_size = 4*1024;
        return 0;
    }

    int32_t index = rootdir_find_file(path + 1); // +1 to cut out char '/'

    if (index >= 0) {
        struct file_entry file;

        rootdir_get_file_entry(index, &file);

        // Default for now
        {
            file_stat->st_mode = S_IFREG | 0777;
            file_stat->st_nlink = 1;
            file_stat->st_size = file.file_size;
            return 0;
        }
    }

    // If no file is found, just create one
    //if (fs_create_file(path + 1) >= 0) {
    //    return 0;
    //}

    return -ENOENT;
}

static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

    TRACE("Called with path = %s, file_stat = %p", path, buf);

    (void) offset;
    (void) fi;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    if (strcmp(path, "/") == 0) {

        filler(buf, "debug", NULL, 0);

        int32_t count = rootdir_get_table_size();

        struct file_entry file;

        for (int i = 0; i < count; i++) {
            if (rootdir_get_file_entry(i, &file) == 0){
                filler(buf, file.name, NULL, 0);
                //printf("   %2i   %3i        %2i   %s \n", i, entry.file_size, entry.data_cluster, entry.name);
            }
        }
    }

    if (strcmp(path, "/debug") == 0) {
        filler(buf, "root_dir_table", NULL, 0);
        filler(buf, "fat_table", NULL, 0);
        //filler(buf, "_cluster_00", NULL, 0);
        //filler(buf, "_cluster_01", NULL, 0);
        //filler(buf, "_cluster_02", NULL, 0);
    }

    return 0;
}

static int fuse_open(const char *path, struct fuse_file_info *file_info) {
    TRACE("Called with path = %s, file_info = %p", path, file_info);
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    TRACE("Called with path = %s, buf = %p, size = %i, offset = %i", path, buf, size, offset);

    if (strcmp(path, "/debug/fat_table") == 0) {
        print_FAT_table();
        return 0;
    }

    if (strcmp(path, "/debug/root_dir_table") == 0) {
        print_root_dir_table();
        return 0;
    }

    {
        // # 1 Find file entry
        int32_t index = rootdir_find_file(path+1);

        if (index < 0) {
            return -ENOENT;
        }

        // # 2) Get data cluster number and size
        struct file_entry entry;

        if (rootdir_get_file_entry(index, &entry) != 0){
            return -ENOENT;
        }

        // # 3) Fetch all cluster data
        for (int i = 0; i < entry.file_size; i++) {
            cluster_read_8bit(entry.data_cluster, i, buf + i);
        }

        return entry.file_size;
    }

    return -ENOENT;
}

static int fuse_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    TRACE("Called with path = %s, buf = %p, size = %i, offset = %i", path, buf, size, offset);

    (void) fi;


    // # 1 Find file entry
    int32_t index = rootdir_find_file(path+1);

    if (index < 0) {
        return -ENOENT;
    }

    struct file_entry file;

    if (rootdir_get_file_entry(index, &file) != 0){
        TRACE("Unable to fetch file entry for index: %i", index);
        return -1;
    }

    int ret = cluster_write_data(file.data_cluster, offset, buf, size);

    if (ret == RET_SUCCESS) {
        ret = rootdir_set_file_size(index, file.file_size + size);
    } else {
        return -ENOSPC;
    }

    return size;
}

static int fuse_unlink(const char *path) {
    if (fs_delete(path+1) == 0) {
        return 0;
    }

    return -ENOENT;
}

static int fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    if (fs_create_file(path + 1) >= 0) {
        return 0;
    }

    return -ENOENT;
}

static int fuse_flush(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    if (fs_create_file(path + 1) >= 0) {
        return 0;
    }

    return -ENOENT;
}


static struct fuse_operations fuse_operations = {
    .getattr = fuse_getattr,
    .open = fuse_open,
    .read = fuse_read,
    .readdir = fuse_readdir,
    .write  = fuse_write,
    .unlink = fuse_unlink,
    .create = fuse_create,
    .flush = fuse_flush,
    .mknod = fuse_mknod,
};

#include "../images/image__cluster_size_100.h"

int main(int argc, char *argv[])
{
    fs_load_ramdisk(DISK_IMAGE);

    fuse_main(argc, argv, &fuse_operations, NULL);

    fs_store_disk_image("fuse_disk_out.img");
}
