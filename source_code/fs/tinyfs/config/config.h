
#define LOG_TO_PRINTF
//#define TRACE_TO_PRINTF


// Cluster positions
#define CLUSTER_FAT_TABLE     (1)
#define CLUSTER_ROOTDIR_TABLE (2)


// NOTE: Defaults are only used when creating new image
//       For existing image files this values are read from cluster 0
#define DEFAULT_CLUSTER_SIZE (100)
#define DEFAULT_CLUSTER_COUNT (30)
