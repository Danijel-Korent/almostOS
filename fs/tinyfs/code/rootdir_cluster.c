#include "rootdir_cluster.h"

#include <stddef.h> // For NULL
#include <string.h> // For strlen, strncmp

#include "util.h"
#include "cluster_operations.h"
#include "FAT_cluster.h"

#include "../config/config.h"


// File entry header
#define ENTRY_IS_USED_OFFSET   (0)
#define FILE_NAME_OFFSET       (0)
#define FILE_SIZE_OFFSET      (13)
#define CLUSTER_NUM_OFFSET    (15)

#define MAX_FILENAME_SIZE     (13)  // 13 bytes "ought to be enough for anybody!
#define SIZE_OF_FILE_ENTRY    (16)

static uint32_t s_cluster_size = 0;

static int32_t rootdir_find_free_index(void);


int rootdir_init(uint32_t cluster_size)
{
    TRACE("Called with cluster_size = %i", cluster_size);

    // TODO: enumerirati sve entry-e u upotrebi i zadnji bajt postaviti na 0 da budem siguran da je NULL terminirano

    if (ERROR(cluster_size == 0)) {
        return -1; // TODO: Search all "return -1" and replace them with names
    }

    s_cluster_size = cluster_size;

    return 0;
}

int32_t rootdir_get_table_size(void)
{
    TRACE("Called");
    int table_size = 0;

    int num_of_rootdir_clusters = get_cluster_chain_size(CLUSTER_ROOTDIR_TABLE);

    if (num_of_rootdir_clusters > 0) {
        if (ERROR(SIZE_OF_FILE_ENTRY == 0)) {
            return -FS_INTERNAL_ERROR; // Prevent div with 0
        }

        table_size = (num_of_rootdir_clusters * s_cluster_size) / SIZE_OF_FILE_ENTRY;
    }

    TRACE("Returning %i", table_size);
    return table_size;
}

static inline uint32_t calculate_file_entry_offset(uint32_t index)
{
    return index * SIZE_OF_FILE_ENTRY; // TODO: Search "index * SIZE_OF_FILE_ENTRY" and replace them this function
}

static int rootdir_set_file_entry(uint32_t index, const char* filename, uint16_t size, uint8_t cluster)
{
    if (ERROR(filename == NULL)) return -1;
    if (ERROR(cluster == 0) || ERROR(cluster == 0xFF)) return -1;
    
    // TODO: Provjeriti da li je entry slobodan??

    int entry_offset = calculate_file_entry_offset(index);

    int filename_len = strlen(filename) + 1; // +1 To include the NULL-terminator

    if (filename_len > MAX_FILENAME_SIZE) {
        filename_len = MAX_FILENAME_SIZE;
    }

    int ret_val = 0;

    ret_val |= cluster_write_data(CLUSTER_ROOTDIR_TABLE,  entry_offset + FILE_NAME_OFFSET, (const uint8_t*)filename, filename_len);

    // TODO: NUll-terminate at end, to be on the safe side
    ret_val |= cluster_write_8bit(CLUSTER_ROOTDIR_TABLE,  entry_offset + FILE_NAME_OFFSET + MAX_FILENAME_SIZE - 1, 0);

    ret_val |= cluster_write_16bit(CLUSTER_ROOTDIR_TABLE, entry_offset + FILE_SIZE_OFFSET, size);
    ret_val |= cluster_write_8bit(CLUSTER_ROOTDIR_TABLE,  entry_offset + CLUSTER_NUM_OFFSET, cluster);

    TRACE("Returning %i", ret_val);
    return ret_val;
}

int rootdir_set_file_size(uint32_t index, uint16_t size)
{
    TRACE("Setting file %i to size %i", index, size);
    int entry_offset = calculate_file_entry_offset(index);

    return cluster_write_16bit(CLUSTER_ROOTDIR_TABLE, entry_offset + FILE_SIZE_OFFSET, size);
}


int32_t rootdir_get_file_entry(uint32_t index, struct file_entry* const entry)
{
    TRACE("Called with index = %i, entry = %p", index, entry);

    if (ERROR(entry == NULL)) return -1;

    int ret = 0;
    int entry_offset = index * SIZE_OF_FILE_ENTRY;

    uint8_t entry_is_used;

    ret |= cluster_read_8bit(CLUSTER_ROOTDIR_TABLE, entry_offset + ENTRY_IS_USED_OFFSET, &entry_is_used);

    if (entry_is_used == 0) {
        return -1;
    }

    // TODO: replicate this in other functions
    ret |= cluster_read_data(CLUSTER_ROOTDIR_TABLE,  entry_offset + FILE_NAME_OFFSET, entry->name, sizeof entry->name);
    ret |= cluster_read_16bit(CLUSTER_ROOTDIR_TABLE, entry_offset + FILE_SIZE_OFFSET, &entry->file_size);
    ret |= cluster_read_8bit(CLUSTER_ROOTDIR_TABLE,  entry_offset + CLUSTER_NUM_OFFSET, &entry->data_cluster);

    //TRACE("Returning %i", ret);
    return ret;
}

int32_t rootdir_find_file(const char* const filename)
{
    TRACE("called with filename = %s", filename);

    if (ERROR(filename == NULL)) return -1;

    int32_t count = rootdir_get_table_size();

    struct file_entry entry;

    for (int i = 0; i < count; i++) {
        if (rootdir_get_file_entry(i, &entry) == 0){
            if (strncmp(filename, (char*)entry.name, sizeof entry.name) == 0) {
                TRACE("File %s found at %i", filename, i);
                return i;
            }
        }
    }

    TRACE("Could not find %s", filename);
    return -1; // TODO replace with name
}


static int rootdir_mark_file_deleted(uint32_t index) {
    TRACE("called with index = %i", index);
    int entry_offset = index * SIZE_OF_FILE_ENTRY; // TODO: stavi u funkciju??

    return cluster_write_8bit(CLUSTER_ROOTDIR_TABLE, entry_offset + ENTRY_IS_USED_OFFSET, 0); // TODO: Replace 0 with "ENTRY_IS_FREE"
}


int rootdir_delete_file(uint32_t index)
{
    // TODO: It doesn't check if rootdir clusters can be freed in case when root directory
    //       has grown into multiple clusters. It's basically a memory leak

    TRACE("called with index = %i", index);
    struct file_entry entry;

    if (rootdir_get_file_entry(index, &entry) != 0){
        return -1;
    }

    if (rootdir_mark_file_deleted(index) == RET_SUCCESS) {
        return free_clusters(entry.data_cluster);
    }

    return -1;
}

// TODO: This code is still ugly
int32_t rootdir_create_file(const char* const filename)
{
    TRACE("called with filename = %s", filename);

    if (ERROR(filename == NULL)) return -1;

    int32_t file_index = rootdir_find_free_index();

    if (file_index < 0) {
        return file_index;
    }

    uint32_t cluster_index = FAT_allocate_cluster();

    // TODO: there are 4 returns in this code, replace them with if's
    if (cluster_index == 0xFF) {
        return -FS_DISK_FULL;
    }

    int ret = rootdir_set_file_entry(file_index, filename, 0, cluster_index);

    if (ret == RET_SUCCESS) {
        ret = file_index;
    } else {
        free_clusters(cluster_index);
    }

    TRACE("Returning %i", ret);
    return ret;
}

static int32_t rootdir_find_free_index(void)
{
    TRACE("called");

    int32_t ret_val = -1;
    int32_t count = rootdir_get_table_size();

    uint8_t entry_is_used;

    for (int i = 0; i < count; i++) {
            int entry_offset = i * SIZE_OF_FILE_ENTRY;

            cluster_read_8bit(CLUSTER_ROOTDIR_TABLE, entry_offset + ENTRY_IS_USED_OFFSET, &entry_is_used);

            if (entry_is_used == 0) {
                TRACE("Found free file index at: %i", i);
                ret_val = i;
                break;
        }
    }

    if (ret_val < 0) {
        // Just return +1 index after last available index in the cluster because writing to
        // unlocated index will automatically allocate new cluster (done by cluster_write_8bit())
        // Current count value is also the index for the first new entry in enlarged table
        ret_val = count;
        TRACE("Ne free index at current size, triggering table expansion");
    }

    TRACE("Returning %i", ret_val);
    return ret_val;
}
