#include "system_calls.h"

#include <stdint.h>

#include "system_headers/unistd.h"  // For STDOUT_FILENO
#include "util.h"
#include "kernel_stdio.h"


// https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/errno-base.h#L18
#define E_BAD_DESCRIPTOR     9
#define E_NO_MEM            12

// Making all system call numbers and arguments compatible with Linux x86-32
// https://elixir.bootlin.com/linux/v6.16/source/arch/x86/entry/syscalls/syscall_32.tbl
#define SYSCALL_RESTART     (0)
#define SYSCALL_EXIT        (1)
#define SYSCALL_WRITE       (4)


void syscall_entry_point_C(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
#if 0
    kernel_printf("\nCalled syscall_entry_point_C() with args: \n");
    kernel_printf("    syscall_num = 0x%08x / %d \n", syscall_num, syscall_num);
    kernel_printf("    arg1 = 0x%08x / %d \n", arg1, arg1);
    kernel_printf("    arg2 = 0x%08x / %d \n", arg2, arg2);
    kernel_printf("    arg3 = 0x%08x / %d \n", arg3, arg3);
#endif

    //sys_write_test();

    if (syscall_num == SYSCALL_WRITE) sys_write((int) arg1, (const void*) arg2, (size_t) arg3);
}

void sys_write_test(void)
{
    char msg[] = "\n\n           >>>> CALLED sys_write_test() <<< \n";

    sys_write(STDOUT_FILENO, msg, sizeof(msg));
}

// Not yet a syscall, just a simple test code at this stage, as the mechanism for invoking syscalls
// is not yet even implemented
ssize_t sys_write(int fd, const void *buf, size_t count)
{
#if 0
    kernel_printf("\nCalled sys_write() with args: \n");
    kernel_printf("    fd = %d \n", fd);
    kernel_printf("    buf = %x \n", buf);
    kernel_printf("    count = %d \n", count);
#endif

    // Currenly we only support write to stdout
    if (fd != STDOUT_FILENO)
    {
        kernel_printf("[ERROR] sys_write(): Bad FD = %d", fd);
        return -E_BAD_DESCRIPTOR;
    }

    unsigned char local_buf[1024] = {0}; // We (obviusly) don't care about performance at this stage

    if (count > sizeof(local_buf)-1)
    {
        kernel_printf("[ERROR] sys_write(): Mesage too big! count = %d", count);
        return -E_NO_MEM;
    }

    mem_copy(local_buf, sizeof(local_buf), buf, count);

    local_buf[sizeof(local_buf)-1] = 0; // Null terminate it

    kernel_printf("%s", local_buf); // I need a function for printing strings

    return count;
}
