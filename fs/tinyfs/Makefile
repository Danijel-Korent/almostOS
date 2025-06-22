
# Define compiler and flags
CC=gcc
CFLAGS= -g -Wall -Iapi -std=c11


# List your source files
SRCS=code/cluster_operations.c code/disk_image.c code/FAT_cluster.c code/fs_operations.c code/rootdir_cluster.c code/util.c

# Define the object files based on the source files
OBJS=$(SRCS:.c=.o)

# Define the SHELL_BIN file name
SHELL_BIN=shell_test


# Default target
all: shell

shell: $(SHELL_BIN)
	./$(SHELL_BIN)


fuse: $(OBJS) tests/fuse_test.c
	$(CC) -Iapi -D_FILE_OFFSET_BITS=64 -o fuse_test $^ -lfuse
	mkdir -p tests/mountpoint_fuse/
	./fuse_test -f -s tests/mountpoint_fuse/  # -d


$(SHELL_BIN): $(OBJS) tests/shell_test.o
	$(CC) $(CFLAGS) -o $@ $^


# Rule to make the object files
%.o: %.c %.h code/util.h config/config.h
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) $(SHELL_BIN) tests/shell_test.o fuse_test


# Prevent make from confusing the clean target with a file name
.PHONY: all clean shell fuse test
