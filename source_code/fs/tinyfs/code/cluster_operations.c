#include "cluster_operations.h"

#include <stddef.h> // for NULL definition

#include "util.h"
#include "FAT_cluster.h"
#include "fs_error_codes.h"


static uint8_t* s_disk_image_ptr = NULL;

uint32_t s_cluster_size = 0;
uint32_t s_cluster_count = 0;

static int get_cluster_at_offset(uint32_t head_cluster_idx, uint32_t offset, uint32_t* target_cluster_idx);
static int allocate_clusters_upto_offset(uint32_t head_cluster_idx, uint32_t offset, uint32_t* target_cluster_idx);


static inline uint32_t calculate_image_offset(uint32_t cluster_idx)
{
    return cluster_idx * s_cluster_size;
}

int cluster_init(uint8_t* const disk_image_ptr, uint32_t cluster_count, uint32_t cluster_size)
{
    TRACE("Called with cluster_count = %i, cluster_size = %i, disk_image_ptr = %p", cluster_count, cluster_size, disk_image_ptr);
    s_disk_image_ptr = disk_image_ptr;
    s_cluster_size = cluster_size;
    s_cluster_count = cluster_count;

    // FAT_init calls cluster_write_8bit() so it must be always initialized after cluster_init()
    return FAT_init(s_cluster_count, s_cluster_size);;
}

// This function doesn't just get offset from head_cluster_idx but iterates over linked clusters
// if the offset is beyond the first cluster
int cluster_read_8bit(uint32_t head_cluster_idx, uint32_t offset, uint8_t* value)
{
    if (ERROR(s_disk_image_ptr == NULL)) return -1;
    if (ERROR(value == NULL)) return -1;

    // TODO: Add argument checking

    uint32_t target_cluster_idx = head_cluster_idx;
    uint32_t target_cluster_offset = offset;

    if (offset >= s_cluster_size) {
        if (ERROR(s_cluster_size == 0)) return -FS_INTERNAL_ERROR; // Prevent div with 0

        target_cluster_offset = offset % s_cluster_size;
        int ret = get_cluster_at_offset(head_cluster_idx, offset, &target_cluster_idx);

        // Couldn't find cluster, bail out!
        if (ret != 0) {
            return ret;
        }
    }

    uint32_t image_offset = calculate_image_offset(target_cluster_idx) + target_cluster_offset;

    //TRACE("Reading image offset: %i \n", image_offset);

    *value = s_disk_image_ptr[image_offset]; // Before setting check if image_offset is smaller than image_size
    //*value = 'A';
    return 0;
}

int cluster_read_16bit(uint32_t head_cluster_idx, uint32_t offset, uint16_t* value)
{
    int ret = 0;
    uint8_t high_val, low_val;

    // Read as little-endian
    ret |= cluster_read_8bit(head_cluster_idx, offset + 0, &high_val);
    ret |= cluster_read_8bit(head_cluster_idx, offset + 1, &low_val);

    *value = (high_val << 8) + low_val;

    return ret;
}

int cluster_read_data(uint32_t head_cluster_idx, uint32_t offset, uint8_t* const buffer_ptr, uint32_t buffer_len)
{
    // TODO: To implement in faster way, without using cluster_read_8bit()
    int ret = 0;

    for (int i = 0; i < buffer_len; i++) {
        ret |= cluster_read_8bit(head_cluster_idx, offset + i, buffer_ptr + i);
    }

    return ret;
}

// This function doesn't just get offset from head_cluster_idx but iterates over linked clusters
// if the offset is beyond the first cluster
//
// NOTE: If offset is beyond all linked clusters, it allocates new clusters up to the specified offset
//
int cluster_write_8bit(uint32_t head_cluster_idx, uint32_t offset, uint8_t value)
{
    // TODO: Add argument checking
    int ret = 0;

    //TRACE("Reading image offset: %i \n", image_offset);

    uint32_t target_cluster_idx    = head_cluster_idx;
    uint32_t target_cluster_offset = offset;

    if (offset >= s_cluster_size) {
        if (ERROR(s_cluster_size == 0)) return -FS_INTERNAL_ERROR; // Prevent div with 0

        target_cluster_offset = offset % s_cluster_size;

        ret = get_cluster_at_offset(head_cluster_idx, offset, &target_cluster_idx);

        if (ret != 0) {
            ret = allocate_clusters_upto_offset(head_cluster_idx, offset, &target_cluster_idx);
        }
    }

    if (ret == 0) {
        uint32_t image_offset = calculate_image_offset(target_cluster_idx) + target_cluster_offset;

        s_disk_image_ptr[image_offset] = value; // Before setting, check if image_offset is smaller than image_size
    }

    return ret;
}

int cluster_write_16bit(uint32_t head_cluster_idx, uint32_t offset, uint16_t value)
{
    uint8_t high_val = value >> 8;
    uint8_t low_val  = value & 0xFF;

    int ret = 0;

    ret |= cluster_write_8bit(head_cluster_idx, offset + 0, high_val);
    ret |= cluster_write_8bit(head_cluster_idx, offset + 1, low_val);

    return ret;
}

int cluster_write_data(uint32_t head_cluster_idx, uint32_t offset, const uint8_t* const buffer_ptr, uint32_t buffer_len)
{
    // TODO: To implement in a faster way, without using cluster_write_8bit()
    int ret = 0;

    for (int i = 0; i < buffer_len; i++) {
        ret |= cluster_write_8bit(head_cluster_idx, offset + i, buffer_ptr[i]);
    }

    return ret;
}

int free_clusters(uint32_t head_cluster)
{
    TRACE("Called with head_cluser = %i", head_cluster);

    if (ERROR(head_cluster == 0) || ERROR(head_cluster == 0xFF)) {
        return -1;
    }

    uint32_t next_cluster = FAT_get_linked_cluster(head_cluster);

    FAT_mark_cluster_as_free(head_cluster);

    // Recursion's exit condition
    if (next_cluster == 0xFF) { // TODO: again magic number
        // Last cluster, nothing to do here anymore
        return 0;
    }

    return free_clusters(next_cluster);
}

int get_cluster_chain_size(uint32_t head_cluster)
{
    TRACE("Called with head_cluser = %i", head_cluster);

    if (ERROR(head_cluster == 0) || ERROR(head_cluster == 0xFF)) {
        return -1;
    }

    int count = 0;

    uint32_t next_cluster = 0xFF;

    do {
        next_cluster = FAT_get_linked_cluster(head_cluster);
        head_cluster = next_cluster;
        count++;
    } while( next_cluster != 0xFF ); // TODO: Hardcoded 0xFF - rename to CLUSTER_CHAIN_END

    TRACE("Returning %i", count);
    return count;
}


// TODO: Move to FAT or move FAT functions here?
static int get_cluster_at_offset(uint32_t head_cluster_idx, uint32_t offset, uint32_t* target_cluster_idx)
{
    //TRACE("Called with head_cluster_idx = %i, offset = %i, targer_cluster_idx_ptr = %p", head_cluster_idx, offset, target_cluster_idx);

    if (ERROR(s_cluster_size == 0)) return -FS_INTERNAL_ERROR; // Prevent div with 0

    int traverse_count = offset / s_cluster_size;
    uint32_t current_cluster = head_cluster_idx;

    for (int i = 0; i < traverse_count; i++) {
        current_cluster = FAT_get_linked_cluster(current_cluster);
    }

    if (current_cluster != 0xFF) { // TODO: Replace hardcoded 0xFF
        *target_cluster_idx = current_cluster;
        return 0;
    }

    return -1;
}

// TODO: Mozda current_cluster preimenovati cluster_iterator??
static int allocate_clusters_upto_offset(uint32_t head_cluster_idx, uint32_t offset, uint32_t* target_cluster_idx)
{
    TRACE("Called with head_cluster_idx = %i, offset = %i, targer_cluster_idx_ptr = %p", head_cluster_idx, offset, target_cluster_idx);

    if (ERROR(s_cluster_size == 0)) return -FS_INTERNAL_ERROR; // Prevent div with 0

    int traverse_count = offset / s_cluster_size;
    uint32_t current_cluster = head_cluster_idx;
    uint32_t next_cluster = 0xFF;

    for (int i = 0; i < traverse_count; i++) {
        next_cluster = FAT_get_linked_cluster(current_cluster);

        // If last cluster in chain
        if (next_cluster == 0xFF) { // TODO: Replace hardcoded 0xFF
            uint32_t new_cluster = FAT_allocate_cluster();

            if (new_cluster == 0xFF) {
                // Disk is full, no point in continuing
                return -FS_DISK_FULL;
            }

            // New cluster successfuly alocated, link it to the current cluster
            FAT_set_linked_cluster(current_cluster, new_cluster);
            next_cluster = new_cluster;
        }

        current_cluster = next_cluster;
    }

    // If we didn't exit from function in 'for' loop, it means that we managed
    // to allocate all clusters up to the cluster pointed by the offset
    *target_cluster_idx = current_cluster;

    return 0;
}
