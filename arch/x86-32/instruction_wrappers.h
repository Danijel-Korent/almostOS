#ifndef _INSTRUCTION_WRAPPERS_H_
#define _INSTRUCTION_WRAPPERS_H_

#include "kernel_stddef.h"

int test_func(int base, int multiplier, int adder);

u8  read_byte_from_IO_port(u16 port_address);
void write_byte_to_IO_port(u16 port_address, u8 data);

u32 get_timestamp(void);
void halt_cpu(void);

u32 get_reg_CS(void);
u32 get_reg_DS(void);
u32 get_reg_SS(void);
u32 get_reg_ES(void);
u32 get_reg_FS(void);
u32 get_reg_GS(void);

void* get_GDT_table(void);


#endif // _INSTRUCTION_WRAPPERS_H_
