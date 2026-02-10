#ifndef _FILESYSTEM_ROUTER_H_
#define _FILESYSTEM_ROUTER_H_

// TODO: Static for now
#define FS_TYPE__NONE    (0)
#define FS_TYPE__TINY_FS (1)
#define FS_TYPE__BLOB_FS (2)
#define FS_TYPE__TEST_FS (3)

void fs_router_init(void);
int mount_fs(int fs_type, const char* directory_path);

int get_list_of_files(const char* dir_name, char* buffer, int buffer_len);
int read_file(const char* file_name, char* buffer, int buffer_len);

#endif
