#ifndef _DISK_INFO_CLUSTER_H_
#define _DISK_INFO_CLUSTER_H_

#include <stdint.h>

int create_disk(void);
int load_ramdisk(uint8_t* const ramdisk_image);
int load_disk(const char* const image_filename);
int store_disk(const char* const image_filename);
void free_disk(void);

unsigned char* get_disk_image_ptr(void);

uint32_t get_disk_cluster_count(void);
uint32_t get_disk_cluster_size(void);


#endif // _DISK_INFO_CLUSTER_H_
