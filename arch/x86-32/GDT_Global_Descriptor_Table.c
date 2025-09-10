#include "GDT_Global_Descriptor_Table.h"

#include <stdint.h>
#include "instruction_wrappers.h"
#include "util.h"

#include "kernel_stdio.h"
#include "cmd_hexdump.h"  // TODO: Should not be here, but will deal with this later

// Docs
//  https://en.wikipedia.org/wiki/Global_Descriptor_Table
//  https://en.wikipedia.org/wiki/Segment_descriptor
//  https://wiki.osdev.org/GDT_Tutorial
//  https://wiki.osdev.org/Segmentation
//  https://wiki.osdev.org/Global_Descriptor_Table



typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdtr_hdr_t;


static uint32_t get_base_address(uint8_t* entry_addr)
{
    uint32_t base = 0;

    uint8_t base_bit_00_07 = entry_addr[2];
    uint8_t base_bit_08_15 = entry_addr[3];
    uint8_t base_bit_16_23 = entry_addr[4];
    uint8_t base_bit_24_31 = entry_addr[7];

    base += base_bit_00_07;
    base += base_bit_08_15 << 8;
    base += base_bit_16_23 << 16;
    base += base_bit_24_31 << 24;

    return base;
}

static uint32_t get_limit(uint8_t* entry_addr)
{
    uint32_t limit = 0;

    uint8_t limit_bit_00_07 = entry_addr[0];
    uint8_t limit_bit_08_15 = entry_addr[1];
    uint8_t limit_bit_16_19 = entry_addr[6] & 0x0F;

    limit += limit_bit_00_07;
    limit += limit_bit_08_15 << 8;
    limit += limit_bit_16_19 << 16;

    return limit;
}

static uint8_t get_access_byte(uint8_t* entry_addr)
{
    // TODO: Replace hardcoded indexes with names in all those functions
    return entry_addr[5];
}

static uint8_t get_flags_nibble(uint8_t* entry_addr)
{
    return entry_addr[6] >> 4;
}

// Descriptor flags
static uint8_t get_flag_granularity(uint8_t flags)
{
    return (flags >> 3) & 0x1;
}

static uint8_t get_flag_size(uint8_t flags)
{
    return (flags >> 2) & 0x1;
}

static uint8_t get_flag_long_mode_code(uint8_t flags)
{
    return (flags >> 1) & 0x1;
}

// Descriptor access byte data
static uint8_t get_descriptor_present_bit(uint8_t access_byte)
{
    return (access_byte >> 7) & 0x1;
}

static uint8_t get_descriptor_privilege_level(uint8_t access_byte)
{
    return (access_byte >> 5) & 0x3;
}

static uint8_t get_descriptor_type(uint8_t access_byte)
{
    return (access_byte >> 4) & 0x1;
}

static uint8_t get_descriptor_executable(uint8_t access_byte)
{
    return (access_byte >> 3) & 0x1;
}

static uint8_t get_descriptor_DC_bit(uint8_t access_byte)
{
    return (access_byte >> 2) & 0x1;
}

static uint8_t get_descriptor_RW_bit(uint8_t access_byte)
{
    return (access_byte >> 1) & 0x1;
}

static uint8_t get_descriptor_accessed_bit(uint8_t access_byte)
{
    return (access_byte >> 0) & 0x1;
}

void print_GDT_table(void)
{
    gdtr_hdr_t* gdtr_hdr = get_GDT_table_location();

    kernel_printf("\nGlobal descriptor table (GDT) data: \n");
    kernel_printf("GDT base  = 0x%x \n", gdtr_hdr->base);
    kernel_printf("GDT limit = %d / 0x%x \n", gdtr_hdr->limit, gdtr_hdr->limit);

    char buf[10] = {0};
    long_to_hex(gdtr_hdr->base, buf, sizeof(buf)-1, 16);
    //kernel_printf("buf = %s \n", buf);
    char* argv[2] = {"", buf};

    kernel_printf("\nGDT table: \n");
    execute__dump_data(2, argv); //TODO: Should not be called here, but will deal with this later

    int num_of_entries = (gdtr_hdr->limit + 1) / 8; // Each GDT entry is 8 bytes long

    kernel_printf("GDT Entries (hex): \n");
    for (int i = 0; i < num_of_entries; i++)
    {
        uint8_t* entry_addr =  (uint8_t*) gdtr_hdr->base + i*8;

        kernel_printf("[%02d] %02X %02X %02X %02X %02X %02X %02X %02X", i*8, entry_addr[0], entry_addr[1], entry_addr[2], entry_addr[3],
                                                                             entry_addr[4], entry_addr[5], entry_addr[6], entry_addr[7]);
        kernel_printf("\n");
    }

    kernel_printf("\nGDT Entries (decoded): \n");
    for (int i = 0; i < num_of_entries; i++)
    {
        uint8_t* entry_addr =  (uint8_t*) gdtr_hdr->base + i*8;

        kernel_printf("\n[%02d] => \n", i*8);

        kernel_printf("     Base address  = 0x%X \n", get_base_address(entry_addr));
        kernel_printf("     Segment limit = 0x%X / %d \n", get_limit(entry_addr), get_limit(entry_addr));

        uint8_t access_byte = get_access_byte(entry_addr);
        kernel_printf("     Access btye [0x%02X]: \n", access_byte);
        kernel_printf("             Present bit           = %d \n", get_descriptor_present_bit(access_byte));
        kernel_printf("             privilege level (DPL) = %d \n", get_descriptor_privilege_level(access_byte));
        kernel_printf("             System Segment bit    = %d \n", get_descriptor_type(access_byte));
        kernel_printf("             Executable bit        = %d \n", get_descriptor_executable(access_byte));
        kernel_printf("             DC bit                = %d \n", get_descriptor_DC_bit(access_byte));
        kernel_printf("             RW bit                = %d \n", get_descriptor_RW_bit(access_byte));
        kernel_printf("             Accessed bit          = %d \n", get_descriptor_accessed_bit(access_byte));

        // TODO: Parse vals
        uint8_t flags = get_flags_nibble(entry_addr);
        kernel_printf("     Flags [0x%X]: \n", flags);
        kernel_printf("             Granularity     = %d \n", get_flag_granularity(flags));
        kernel_printf("             Size flag       = %d \n", get_flag_size(flags));
        kernel_printf("             Long-mode code  = %d \n", get_flag_long_mode_code(flags));
    }

    kernel_printf("\nSegment registers: \n");
    kernel_printf("CS = %02d / 0x%02X \n", get_reg_CS(), get_reg_CS());
    kernel_printf("DS = %02d / 0x%02X \n", get_reg_DS(), get_reg_DS());
    kernel_printf("SS = %02d / 0x%02X \n", get_reg_SS(), get_reg_SS());
    kernel_printf("ES = %02d / 0x%02X \n", get_reg_ES(), get_reg_ES());
    kernel_printf("FS = %02d / 0x%02X \n", get_reg_FS(), get_reg_FS());
    kernel_printf("GS = %02d / 0x%02X \n", get_reg_GS(), get_reg_GS());
}

/*

How the GRUB's table looks like

mem dump:
00008F5C: F0 FF 00 00 00 00 00 00  FF FF 00 00 00 9A CF 00   |................|
00008F6C: FF FF 00 00 00 93 CF 00  FF FF 00 00 00 9E 00 00   |................|
00008F7C: FF FF 00 00 00 93 00 00  27 00 5C 8F 00 00 90 90   |........'.\.....|

Entries:
00: F0 FF 00 00 00 00 00 00 (null entry)
08: FF FF 00 00 00 9A CF 00 (Executable)
16: FF FF 00 00 00 93 CF 00 (Non-executable)
24: FF FF 00 00 00 9E 00 00 (16-bit protected mode segment - Executable)
32: FF FF 00 00 00 93 00 00 (16-bit protected mode segment - Non-executable)



       FL AA
    B4 03 BB B3 B1 B0 L1 L0
08: 00 CF 9A 00 00 00 FF FF (wikipedia's table order)

B0-4 - Base address
L0-3 - Limit
F0   - Flags
AB   - Access byte

*/
