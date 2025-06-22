#ifndef _FAT_CLUSTER_H_
#define _FAT_CLUSTER_H_

#include <stdint.h>

int FAT_init(uint32_t cluster_count, uint32_t cluster_size);

uint32_t FAT_allocate_cluster(void);
uint32_t FAT_get_linked_cluster(uint32_t cluster_head); // TODO: rename to get_linked_cluster() ???
void     FAT_set_linked_cluster(uint32_t cluster_head, uint32_t next_cluster);

int FAT_mark_cluster_as_used(uint32_t cluster_index);
int FAT_mark_cluster_as_free(uint32_t cluster_index);


#endif // _FAT_CLUSTER_H_
