#ifndef _FS_OPERATIONS_H_
#define _FS_OPERATIONS_H_

#include <stdint.h>
#include "fs_error_codes.h"

int fs_load_ramdisk(uint8_t* const ramdisk_image);
int fs_load_disk_image(const char* const image_filename);
int fs_store_disk_image(const char* const image_filename);

int fs_create_file(const char* const filename);
int fs_delete(const char* const filename);
int fs_open(const char* const filename);

int fs_write(int handler, const uint8_t* buffer_ptr, uint32_t buffer_len);
int fs_close(int handler);


#endif // _FS_OPERATIONS_H_
