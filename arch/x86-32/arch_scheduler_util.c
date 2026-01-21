#include "arch_scheduler.h"
#include "kernel_stdio.h"


void print_process_ctx(struct process_ctx* process)
{
    kernel_printf("> Process context: \n");
    kernel_printf("    Name = %s \n", process->name);
    kernel_printf("    Ptr  = 0x%x \n\n", process);

    kernel_printf("    IP = 0x%x \n", process->reg_ip);
    kernel_printf("    SP = 0x%x \n", process->reg_esp);
    kernel_printf("    BP = 0x%x \n\n", process->reg_ebp);

    kernel_printf("    A  = 0x%x \n", process->reg_eax);
    kernel_printf("    B  = 0x%x \n", process->reg_ebx);
    kernel_printf("    C  = 0x%x \n", process->reg_ecx);
    kernel_printf("    D  = 0x%x \n", process->reg_edx);
    kernel_printf("    SI = 0x%x \n", process->reg_esi);
    kernel_printf("    DI = 0x%x \n\n", process->reg_edi);

    kernel_printf("    Flags  = 0x%x \n\n", process->reg_eflags);

    u32* sp_ptr = (u32*) process->reg_esp;

    kernel_printf("    Stack \n", process->reg_eflags);
    kernel_printf("      0x%08x: 0x%08x   \n", sp_ptr, *sp_ptr); sp_ptr++;
    kernel_printf("      0x%08x: 0x%08x   \n", sp_ptr, *sp_ptr); sp_ptr++;
    kernel_printf("      0x%08x: 0x%08x   \n", sp_ptr, *sp_ptr); sp_ptr++;
    kernel_printf("      0x%08x: 0x%08x   \n", sp_ptr, *sp_ptr); sp_ptr++;
    kernel_printf("      0x%08x: 0x%08x \n\n", sp_ptr, *sp_ptr); sp_ptr++;
}
