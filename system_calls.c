#include "system_calls.h"

#include "system_headers/unistd.h"  // For STDOUT_FILENO
#include "util.h"
#include "kernel_stdio.h"


// https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/errno-base.h#L18
#define E_BAD_DESCRIPTOR     9
#define E_NO_MEM            12


// Not yet a syscall, just a simple test code at this stage, as the mechanism for invoking syscalls
// is not yet even implemented
ssize_t sys_write(int fd, const void *buf, size_t count)
{
    // Currenly we only support write to stdout
    if (fd != STDOUT_FILENO)
    {
        return -E_BAD_DESCRIPTOR;
    }

    unsigned char local_buf[1024] = {0}; // We (obviusly) don't care about performance at this stage

    if (count > sizeof(local_buf)-1)
    {
        return -E_NO_MEM;
    }

    mem_copy(local_buf, sizeof(local_buf), buf, count);

    local_buf[sizeof(local_buf)-1] = 0; // Null terminate it

    kernel_printf("%s", local_buf); // I need a function for printing strings

    return count;
}
