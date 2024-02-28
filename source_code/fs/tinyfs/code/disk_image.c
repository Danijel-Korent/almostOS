#include "disk_image.h"

#include <stddef.h> // For size_t, NULL
#include <stdio.h>
#include <stdlib.h> // For malloc, free

#include "../config/config.h"
#include "util.h"
#include "FAT_cluster.h"
#include "cluster_operations.h"
#include "rootdir_cluster.h"


static uint8_t* s_disk_image_ptr = 0;
static uint32_t s_disk_image_size = 0;

static uint8_t  s_cluster_count = DEFAULT_CLUSTER_COUNT;
static uint16_t s_cluster_size  = DEFAULT_CLUSTER_SIZE;

static uint8_t* load_file_into_memory(const char *filename, uint32_t *size);


// TODO: Da li ovo igdje koristim?
unsigned char* get_s_disk_image_ptr(void)
{
    return s_disk_image_ptr;
}

uint8_t  disk_get_cluster_count(void)
{
    return s_cluster_count;
}

uint16_t disk_get_cluster_size(void)
{
    return s_cluster_size;
}

int create_disk(void)
{
    TRACE("Called");
    LOG("Creating new disk... \n");

    s_cluster_count = DEFAULT_CLUSTER_COUNT;
    s_cluster_size  = DEFAULT_CLUSTER_SIZE;

    s_disk_image_size = s_cluster_count * s_cluster_size;

    s_disk_image_ptr = malloc(s_disk_image_size);

    if (s_disk_image_ptr == NULL) {
        return -FS_NO_MEMORY;
    }

    TRACE("Created image of size %i, s_cluster_count = %i, s_cluster_size = %i", s_disk_image_size, s_cluster_count, s_cluster_size);
    // Cluster 0 -> Set volume name
    // Cluster 0 -> Set cluster_count
    // Cluster 0 -> Set cluster_size

    // TODO: check return values
    // TODO: Move to better place
    // TODO: Duplicated code
    // TODO: Idealno bi bilo da bude u fs_load_disk_image() -> ali onda mora proslijediti van informacije o cluster_count, cluster_size ili dodati funkcije za dohvacanje
    cluster_init(s_disk_image_ptr, s_cluster_count, s_cluster_size);
    rootdir_init(s_cluster_size);

    return 0;
}


int load_ramdisk(uint8_t* const ramdisk_image)
{
    TRACE("Called with image_filename: %p", ramdisk_image);
    LOG("Loading ramdisk image \n");

    if (ramdisk_image == NULL) {
        return -FS_NO_MEMORY;
    }

    s_cluster_count = DEFAULT_CLUSTER_COUNT;
    s_cluster_size  = DEFAULT_CLUSTER_SIZE;

    s_disk_image_size = s_cluster_count * s_cluster_size;

    s_disk_image_ptr = ramdisk_image;

    TRACE("Created image of size %i, s_cluster_count = %i, s_cluster_size = %i", s_disk_image_size, s_cluster_count, s_cluster_size);
    // Cluster 0 -> Set volume name
    // Cluster 0 -> Set cluster_count
    // Cluster 0 -> Set cluster_size

    // TODO: check return values
    // TODO: Move to better place
    // TODO: Duplicated code
    cluster_init(s_disk_image_ptr, s_cluster_count, s_cluster_size);
    rootdir_init(s_cluster_size);

    return 0;
}

int load_disk(const char* const image_filename)
{
    TRACE("Called with image_filename: %s", image_filename);
    LOG("Loading image: %s \n", image_filename);

    // TODO: Hardcoded, read from cluster 0
    s_cluster_count = DEFAULT_CLUSTER_COUNT;
    s_cluster_size  = DEFAULT_CLUSTER_SIZE;

    s_disk_image_ptr = load_file_into_memory(image_filename, &s_disk_image_size);

    if (s_disk_image_ptr == NULL) return -FS_NO_IMAGE_FILE;

    // TODO: check return values
    // TODO: Move to better place
    // TODO: Duplicated code
    cluster_init(s_disk_image_ptr, s_cluster_count, s_cluster_size);
    rootdir_init(s_cluster_size);

    LOG("Loading image: SUCCESS \n");

    return RET_SUCCESS;
}

int store_disk(const char* const image_filename)
{
    TRACE("Called with image_filename: %s", image_filename);
    TRACE("Disk image size: %i", s_disk_image_size);

    if (ERROR(s_disk_image_ptr == NULL)) return -1;

    FILE *file = fopen(image_filename, "wb");
    if (file == NULL) {
        LOG("Storing image: failed to open the file \n");
        return -1;
    }

    size_t written = fwrite(s_disk_image_ptr, sizeof(uint8_t), s_disk_image_size, file);
    if (written != s_disk_image_size) {
        LOG("Storing image: Error writing to file \n");
        fclose(file);
        return -1;
    }

    fclose(file);
    LOG("Storing image: successfully written to %s \n", image_filename);

    return 0;
}

void free_disk(void)
{
    free(s_disk_image_ptr);
    s_disk_image_ptr = NULL;

    s_cluster_count = DEFAULT_CLUSTER_COUNT;
    s_cluster_size  = DEFAULT_CLUSTER_SIZE;
}



static uint8_t* load_file_into_memory(const char *filename, uint32_t *size) {
    FILE *file;
    uint8_t *buffer;

    TRACE("Called with name = %s, size_ptr = %p", filename, size);

    file = fopen(filename, "rb");
    if (file == NULL) {
        LOG("Failed to load file %s! Did you try to clean the tape? \n", filename);
        return NULL;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    TRACE("File size = %i", *size);

    if (*size == 0)
    {
        LOG("[ERROR] File is empty! \n");
        fclose(file);
        return NULL;
    }

    buffer = malloc(*size); // TODO: Check the free() normal and error paths
    if (buffer == NULL) {
        LOG("[ERROR] Error allocating memory \n");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, *size, file) != *size) {
        LOG("[ERROR] Error reading file \n");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return buffer;
}
