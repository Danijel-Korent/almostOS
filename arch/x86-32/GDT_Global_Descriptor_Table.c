#include "GDT_Global_Descriptor_Table.h"

#include <stdint.h>
#include "instruction_wrappers.h"
#include "util.h"

#include "kernel_stdio.h"
#include "cmd_hexdump.h"  // TODO: Should not be here, but will deal with this later


typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr_hdr_t;



void print_GDT_table(void)
{
    gdtr_hdr_t* gdtr_hdr = get_GDT_table();

    kernel_printf("\nGlobal descriptor table (GDT) data: \n");
    kernel_printf("GDT base  = %x \n", gdtr_hdr->base);
    kernel_printf("GDT limit = %x \n", gdtr_hdr->limit);

    char buf[10] = {0};
    long_to_hex(gdtr_hdr->base, buf, sizeof(buf)-1, 16);
    //kernel_printf("buf = %s \n", buf);
    char* argv[2] = {"", buf};

    kernel_printf("\nGDT table: \n");
    //TODO: Should not be called here, but will deal with this later
    execute__dump_data(2, argv);

    kernel_printf("Segment registers: \n");
    kernel_printf("CS = %x \n", get_reg_CS());
    kernel_printf("DS = %x \n", get_reg_DS());
    kernel_printf("SS = %x \n", get_reg_SS());
    kernel_printf("ES = %x \n", get_reg_ES());
    kernel_printf("FS = %x \n", get_reg_FS());
    kernel_printf("GS = %x \n", get_reg_GS());
}
