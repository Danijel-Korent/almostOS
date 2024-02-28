#ifndef _CLUSTER_OPERATIONS_H_
#define _CLUSTER_OPERATIONS_H_

#include <stdint.h>

int cluster_init(unsigned char* const disk_image_ptr, uint32_t cluster_count, uint32_t cluster_size);

// TOOD: Rename all write_to_cluster_XYbit()
int cluster_write_data(uint32_t cluster_no, uint32_t offset, const uint8_t* const buffer_ptr, uint32_t buffer_len);
int cluster_write_8bit(uint32_t cluster_no, uint32_t offset, uint8_t value);
int cluster_write_16bit(uint32_t cluster_no, uint32_t offset, uint16_t value);

// TODO: Rename all read_from_cluster_XYbit()
int cluster_read_data(uint32_t cluster_no, uint32_t offset, uint8_t* const buffer_ptr, uint32_t buffer_len);
int cluster_read_8bit(uint32_t cluster_no, uint32_t offset, uint8_t* value);
int cluster_read_16bit(uint32_t cluster_no, uint32_t offset, uint16_t* value);

int get_cluster_chain_size(uint32_t head_cluster);
int free_clusters(uint32_t head_cluster);

#endif // _CLUSTER_OPERATIONS_H_
