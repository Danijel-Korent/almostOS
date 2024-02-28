#include "FAT_cluster.h"

#include "../config/config.h"
#include "util.h"
#include "cluster_operations.h"


static uint32_t s_cluster_count = 0;
static uint32_t s_cluster_size  = 0;

// TODO: Merge with cluster_operations.c
// TODO: switch to direct access by pointer. Using cluster_read_8bit() is wasteful, unnecessary and creates circular dependency

int FAT_init(uint32_t cluster_count, uint32_t cluster_size)
{
    TRACE("Called with cluster_count = %i, cluster_size = %i", cluster_count, cluster_size);

    // TODO: add sanity check (if cluster 0 i 1 are 0xFF, provjeriti da FAT tablica nema circular dep i duple linkove)

    s_cluster_size = cluster_size;
    s_cluster_count = cluster_count;

    FAT_mark_cluster_as_used(0);
    FAT_mark_cluster_as_used(CLUSTER_FAT_TABLE);

    uint32_t root_cluster_val = FAT_get_linked_cluster(CLUSTER_ROOTDIR_TABLE);

    if (root_cluster_val == 0) {
        FAT_mark_cluster_as_used(CLUSTER_ROOTDIR_TABLE);
    }

    return 0;
}

uint32_t FAT_get_linked_cluster(uint32_t cluster_head)
{
    if (ERROR(cluster_head == 0) || ERROR(cluster_head == 0xFF)) {
        return 0xFF;
    }

    uint8_t next_cluster_value = 0xFF;

    // TODO: Since FAT table doesnt spill over to more cluster - replace with direct ptr access
    cluster_read_8bit(CLUSTER_FAT_TABLE, cluster_head, &next_cluster_value);

    return next_cluster_value;
}

void FAT_set_linked_cluster(uint32_t cluster_head, uint32_t next_cluster)
{
    // TODO: Since FAT table doesnt spill over to more cluster - replace with direct ptr access
    cluster_write_8bit(CLUSTER_FAT_TABLE, cluster_head, next_cluster);
}

// TODO: Move to cluster operations??
// Finds first free cluster and marks it as used
uint32_t FAT_allocate_cluster(void)
{
    for (int index = 3; index < s_cluster_count; index++) {
        TRACE("Checking FAT Index: %i", index);

        uint32_t next_cluster_value = FAT_get_linked_cluster(index);

        if (next_cluster_value == 0) {
            TRACE("Found free cluster at FAT index: %i", index);
            FAT_mark_cluster_as_used(index);
            return index;
        }
    }

    TRACE("Unable to find free FAT index!");
    return 0xFF; // TODO: Replace hardcoded 0xFF with name
}

int FAT_mark_cluster_as_used(uint32_t cluster_index)
{
    TRACE("Cluster %i marked as used", cluster_index);
    return cluster_write_8bit(CLUSTER_FAT_TABLE, cluster_index, 0xFF); // TODO: Replace 0xFF with name
}

int FAT_mark_cluster_as_free(uint32_t cluster_index)
{
    TRACE("Cluster %i marked as used", cluster_index);
    return cluster_write_8bit(CLUSTER_FAT_TABLE, cluster_index, 0x00); // TODO: Replace 0xFF with name
}
