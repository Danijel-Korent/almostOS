#ifndef _VIRTUAL_FS_H_
#define _VIRTUAL_FS_H_

int vfs_get_list_of_files(const char* directory_path, char* buffer, int buffer_len);
int vfs_read_file(const char* file_path, char* buffer, int buffer_len);


#endif
