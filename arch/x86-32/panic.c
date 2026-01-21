#include "panic.h"

#include "kernel_stdio.h"
#include "instruction_wrappers.h"

/*
// For fetching registers, if I will implement it
struct cpu_registers
{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 esi;
    u32 edi;
    u32 esp;
    u32 ebp;
    u32 ip;     // PC
    u32 eflags;

    // Segment regs
    u32 cs;
    u32 ds;
    u32 ss;
    u32 es;
    u32 fs;
    u32 gs;
};
*/

void time_to_die(void)
{
    kernel_printf("\n\n\n ---------------------------/ KERNEL PANIC /--------------------------- \n");

    kernel_printf("\nSegment registers: \n");
    kernel_printf("CS = %02d / 0x%02X \n", get_reg_CS(), get_reg_CS());
    kernel_printf("DS = %02d / 0x%02X \n", get_reg_DS(), get_reg_DS());
    kernel_printf("SS = %02d / 0x%02X \n", get_reg_SS(), get_reg_SS());
    kernel_printf("ES = %02d / 0x%02X \n", get_reg_ES(), get_reg_ES());
    kernel_printf("FS = %02d / 0x%02X \n", get_reg_FS(), get_reg_FS());
    kernel_printf("GS = %02d / 0x%02X \n", get_reg_GS(), get_reg_GS());

    // Print return address

    // Print stack

    // Does it make sense to print other registers?

    while(1);
}
