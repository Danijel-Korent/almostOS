#include <stdio.h>

#include "fs_operations.h"

#include "../code/util.h"
#include "../code/disk_image.h"
#include "../code/rootdir_cluster.h"
#include "../code/cluster_operations.h"
#include "../code/FAT_cluster.h"

static void open_and_fill_file(const char* name, int count, const char* content_ptr, int content_size);

void command_cat(const char* filename)
{
    if (ERROR(filename == NULL)) return;

    printf("\n");
    printf("> cat %s", filename);
    printf("\n");


    // # 1 Find file entry
    int32_t index = rootdir_find_file(filename);

    if (index < 0) {
        printf("cat> File not found!! \n");
        return;
    }


    // # 2) Get data cluster number and size
    struct file_entry entry;

    if (rootdir_get_file_entry(index, &entry) != 0){
        return;
    }

    // 

    // # 3) Fetch all cluster data

    for (int i = 0; i<entry.file_size; i++) {

        uint8_t byte;

        cluster_read_8bit(entry.data_cluster, i, &byte);

        if (byte >= 0) {

            if (byte < 32) byte = '.';

            putchar (byte);
        }
    }

    printf("\n\n");
}


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
    printf("Allocated FAT clusters:\n");

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

void command_ls_root(void)
{
    int32_t count = rootdir_get_table_size();

    struct file_entry entry;

    printf("> ls /");
    printf("\n\n");
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

void command_rm(const char* filename)
{
    printf("> rm %s \n", filename);
    fs_delete(filename);
    command_ls_root();
}

void command_create_file(const char* filename)
{
    printf("> open %s \n", filename);
    fs_create_file(filename);
    command_ls_root();
}


int main(void)
{
    int32_t ret = -1;

    printf("\nShell starting!\n");

    //ret = fs_load_disk_image("disk_image_in.img");
    ret = fs_load_disk_image(NULL);

    if (ret < 0) {
        printf("Error returned by fs_load_disk_image()!\n");
        return 1;
    }

    command_ls_root();

#if 1 // ALL COMMANDS

#if 0
    command_ls_root();
    command_cat("Test_1");
    command_cat("Test_2");
    command_cat("Test_3");
    command_cat(NULL);
    command_cat("a.out");
#endif

#if 0
    command_rm("Test_22");
    command_rm("Test_2");
    command_rm("Test_1");
#endif

#if 1
    command_create_file("new_file_01");
    command_create_file("new_file_02");
    command_create_file("new_file_03");
    command_rm("Test_10");
    command_create_file("new_file_04");
#endif

#if 1
    command_cat("new_file_04");
    {
        int handle = fs_open("new_file_04");

        if (handle >= 0) {
            int ret;
            char msg[] = "#__A write to new_file_04 file!";

            for (int i = 0; i < 2; i++) {
                ret = fs_write(handle, (uint8_t*)msg, sizeof msg -1);
                if (ret != 0) {
                    printf("fs_write() failed! ");
                }
            }
        }
    }

    command_cat("new_file_04");
    command_cat("a.out");
    int handle = fs_open("a.out");

    if (handle >= 0) {
        int ret;
        char msg[] = "0123456789";

        for (int i = 0; i < 15; i++) {
            ret = fs_write(handle, (uint8_t*)msg, sizeof msg -1);
            if (ret != 0) {
                printf("fs_write() failed! ");
            }
        }
    }

    command_cat("a.out");
    command_cat("new_file_04");
    command_ls_root();
#endif

#if 1
    command_cat("Test_210");
    //command_ls_root();
    command_rm("Test_210");
    command_create_file("new_file_05");
    command_rm("a.out");
    command_create_file("new_file_06");

    command_rm("new_file_04");
    command_rm("new_file_01");
    command_rm("new_file_05");
    command_rm("Test_30");
    command_rm("new_file_02");
    command_rm("new_file_06");

    command_create_file("new_file_001");
    command_create_file("new_file_002");
    command_create_file("new_file_003");
    command_create_file("new_file_004");
    command_create_file("new_file_005");
    command_create_file("new_file_006");
    command_create_file("new_file_007");
    command_create_file("new_file_008");
    command_create_file("new_file_009");
    command_create_file("new_file_010");
    command_create_file("new_file_011");
    command_create_file("new_file_012");
    command_create_file("new_file_013");
    command_create_file("new_file_014");
    command_create_file("new_file_015");
    command_create_file("new_file_016");
    command_create_file("new_file_017");
    command_create_file("new_file_018");
    command_create_file("new_file_019");

    print_FAT_table();
#endif

#endif // ALL COMMANDS

    ret = fs_store_disk_image("disk_image_out.img");

    return 0;
}

static void open_and_fill_file(const char* name, int count, const char* content_ptr, int content_size)
{
    int handle = fs_open(name);

    if (handle >= 0) {
        int ret;

        for (int i = 0; i < count; i++) {
            ret = fs_write(handle, (uint8_t*)content_ptr, content_size);

            if (ret != 0) {
                printf("fs_write() failed! ");
            }
        }
    }
}