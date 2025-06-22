#ifndef _ROOTDIR_CLUSTER_H_
#define _ROOTDIR_CLUSTER_H_

#include <stdint.h>
#include "fs_error_codes.h"


struct file_entry {
    union {
        uint8_t is_used;
        uint8_t name[13]; // This could be just a pointer if we limit name length and cluster length in a way
                       // that "name" never get split between two clusters. Then we could just return a pointer
                       // to the data in the disk image
    };
    uint8_t  data_cluster;
    uint16_t file_size;
};

int rootdir_init(uint32_t cluster_size);

int32_t rootdir_find_file(const char* const filename); // TODO: Preimenovati u rootdir_get_file_index()?
int32_t rootdir_create_file(const char* const filename);
int32_t rootdir_get_file_entry(uint32_t index, struct file_entry* const entry); // TODO: preimenovati u rootdir_get_file()
int32_t rootdir_get_table_size(void); // TODO: Preimenovati u rootdir_get_table_capacity()?


int rootdir_set_file_size(uint32_t index, uint16_t size);

int rootdir_delete_file(uint32_t index);
//int rootdir_mark_file_deleted(uint32_t index);


#endif // _ROOTDIR_CLUSTER_H_
