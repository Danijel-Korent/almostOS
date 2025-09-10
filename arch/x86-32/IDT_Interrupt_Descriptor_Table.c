#include "IDT_Interrupt_Descriptor_Table.h"

#include <stdint.h>
#include "instruction_wrappers.h"

#include "util.h"
#include "kernel_stdio.h"
#include "cmd_hexdump.h"  // TODO: Should not be here, but will deal with this later

// https://en.wikipedia.org/wiki/Interrupt_descriptor_table
// https://wiki.osdev.org/Interrupt_Descriptor_Table


#define VEC_DIVIDE_ERROR        (0x0)    // For example division by zero
#define VEC_INVALID_OPCODE      (0x6)    // Invalid instruction
#define VEC_SEGMENT_NOT_PRESENT (0xB)    // GDT entry not set?  -> !!! SETS ERROR CODE !!!
#define VEC_GENERAL_PROTECTION  (0xD)    // Trigger to any entry not set/present will trigger this fault -> !!! SETS ERROR CODE !!!
#define VEC_LINUX_SYSCALL_API   (0x80)   // Default syscall vector on 32-bit Linux


#define IDT_ENTRY_SIZE             (8)   // In bytes
#define IDT_MAX_NUM_OF_ENTRIES   (256)

#define GATE_TYPE__TASK_GATE        (0x05)
#define GATE_TYPE__16_BIT_INTERRUPT (0x06)
#define GATE_TYPE__16_BIT_TRAP      (0x07)
#define GATE_TYPE__32_BIT_INTERRUPT (0x0E)
#define GATE_TYPE__32_BIT_TRAP      (0x0F)


typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_hdr_t;

static uint8_t IDT_table[IDT_MAX_NUM_OF_ENTRIES * IDT_ENTRY_SIZE] = {0};


static void add_IDT_entry(idt_hdr_t* idt_hdr, uint8_t vector_id, uint8_t privilage, uint8_t gate_type, uint16_t segment, void* func_handler);

static uint8_t* get_IDT_table_entry(idt_hdr_t* idt_hdr, uint8_t index);

static uint32_t get_descriptor_offset      (uint8_t* entry_addr);
static uint16_t get_descriptor_segment     (uint8_t* entry_addr);
static uint8_t  get_descriptor_gate_type   (uint8_t* entry_addr);
static uint32_t get_descriptor_privilege   (uint8_t* entry_addr);
static uint8_t  get_descriptor_present_bit (uint8_t* entry_addr);

static void set_descriptor_offset      (uint8_t* entry_addr, uint32_t offset_addr);
static void set_descriptor_segment     (uint8_t* entry_addr, uint16_t segement_selector);
static void set_descriptor_gate_type   (uint8_t* entry_addr, uint8_t gate_type);
static void set_descriptor_privilege   (uint8_t* entry_addr, uint8_t CPU_privilege);
static void set_descriptor_present_bit (uint8_t* entry_addr, uint8_t value);



void test_INT_HANDLER_00_DIVIDE_ERROR(void)
{
    kernel_printf("\n                             !!! CALLED INT_HANDLER_00 (Divide error) !!! \n");
    while(1);
}

void test_INT_HANDLER_06_INVALID_OPCODE(void)
{
    kernel_printf("\n                             !!! CALLED INT_HANDLER_06 (Invalid opcode) !!! \n");
    while(1);
}

void test_INT_HANDLER_0B_SEGMENT_NOT_PRESENT(void)
{
    kernel_printf("\n                             !!! CALLED INT_HANDLER_0B (Segement not present) !!! \n");
    while(1);
}

void test_INT_HANDLER_0D_GENERAL_PROTECTION(void)
{
    kernel_printf("\n                             !!! CALLED INT_HANDLER_0D (General Protection Fault) !!! \n");
    while(1);
}

void test_INT_HANDLER_80_LINUX_SYSCALL(void)
{
    kernel_printf("\n                             !!! CALLED INT_HANDLER_80 (Linux Syscall) !!! \n");
}

void configure_interrupt_descriptor_table(void)
{
    kernel_printf("\nCalled configure_interrupt_descriptor_table(): \n");

    idt_hdr_t* idt_hdr = get_IDT_table_location();

    kernel_printf("\nOld IDT data: \n");
    kernel_printf("IDT base  = 0x%x \n", idt_hdr->base);
    kernel_printf("IDT limit = %d / 0x%x \n", idt_hdr->limit, idt_hdr->limit);

    int num_of_entries = (idt_hdr->limit + 1) / IDT_ENTRY_SIZE;

    kernel_printf("Number of entries = %d / 0x%x \n", num_of_entries, num_of_entries);

    // Set new table location
    idt_hdr->base = (uint32_t) IDT_table;
    idt_hdr->limit = sizeof(IDT_table);

    kernel_printf("\nNew IDT table location = 0x%x \n", idt_hdr->base);
    kernel_printf("New IDT table limit = %d / 0x%x \n", idt_hdr->limit, idt_hdr->limit);

    // Delete all entries
    for (int i = 0; i < (idt_hdr->limit / IDT_ENTRY_SIZE); i++)
    {
        uint64_t* entry_addr_64 = (uint64_t*) get_IDT_table_entry(idt_hdr, i);

        *entry_addr_64 = 0;
    }

    // TODO: Replace with a function like this: add_IDT_entry(VEC_DIVIDE_ERROR, privilage, gate_type, segment, func_handler)
    // Vector 0 - VEC_DIVIDE_ERROR
    add_IDT_entry(idt_hdr, VEC_DIVIDE_ERROR, /*Privilege*/ 0, GATE_TYPE__32_BIT_TRAP, /*Segment*/ 0x8, test_INT_HANDLER_00_DIVIDE_ERROR);
    add_IDT_entry(idt_hdr, VEC_INVALID_OPCODE, /*Privilege*/ 0, GATE_TYPE__32_BIT_TRAP, /*Segment*/ 0x8, test_INT_HANDLER_06_INVALID_OPCODE);
    add_IDT_entry(idt_hdr, VEC_SEGMENT_NOT_PRESENT, /*Privilege*/ 0, GATE_TYPE__32_BIT_TRAP, /*Segment*/ 0x8, test_INT_HANDLER_0B_SEGMENT_NOT_PRESENT);
    add_IDT_entry(idt_hdr, VEC_GENERAL_PROTECTION, /*Privilege*/ 0, GATE_TYPE__32_BIT_TRAP, /*Segment*/ 0x8, test_INT_HANDLER_0D_GENERAL_PROTECTION);
    add_IDT_entry(idt_hdr, VEC_LINUX_SYSCALL_API , /*Privilege*/ 0, GATE_TYPE__32_BIT_INTERRUPT, /*Segment*/ 0x8, test_INT_HANDLER_80_LINUX_SYSCALL);
    // add_IDT_entry(idt_hdr, vector_ID , /*Privilege*/ 0, gate, /*Segment*/ 0x8, func);

    set_IDT_table_location();
}


void print_interrupt_descriptor_table(void)
{
    idt_hdr_t* idt_hdr = get_IDT_table_location();

    kernel_printf("\nGlobal descriptor table (IDT) data: \n");
    kernel_printf("IDT base  = 0x%x \n", idt_hdr->base);
    kernel_printf("IDT limit = %d / 0x%x \n", idt_hdr->limit, idt_hdr->limit);

    int num_of_entries = (idt_hdr->limit + 1) / IDT_ENTRY_SIZE;

    kernel_printf("Number of entries = %d / 0x%x \n", num_of_entries, num_of_entries);

#if 0
    char buf[10] = {0};
    long_to_hex(idt_hdr->base, buf, sizeof(buf)-1, 16);
    //kernel_printf("buf = %s \n", buf);
    char* argv[2] = {"", buf};

    kernel_printf("\nIDT table: \n");
    execute__dump_data(2, argv); //TODO: Should not be called here, but will deal with this later
#endif

#if 1
    kernel_printf("\nIDT Entries (hex): \n");
    for (int i = 0; i < num_of_entries; i++)
    {
        uint8_t* entry_addr =  (uint8_t*) idt_hdr->base + i * IDT_ENTRY_SIZE;

        if (get_descriptor_present_bit(entry_addr))
        {
            kernel_printf("[%03d] %02X %02X %02X %02X %02X %02X %02X %02X (BE) \n", i, entry_addr[0], entry_addr[1], entry_addr[2], entry_addr[3],
                                                                                    entry_addr[4], entry_addr[5], entry_addr[6], entry_addr[7]);

            //kernel_printf("[%02d] %02X %02X %02X %02X %02X %02X %02X %02X (LE) \n", i, entry_addr[7], entry_addr[6], entry_addr[5], entry_addr[4],
            //                                                                        entry_addr[3], entry_addr[2], entry_addr[1], entry_addr[0]);
        }
    }
#endif

#if 1
    kernel_printf("\nIDT Entries (decoded): \n");
    for (int i = 0; i < num_of_entries; i++)
    {
        uint8_t* entry_addr =  (uint8_t*) idt_hdr->base + i * IDT_ENTRY_SIZE;

        if (get_descriptor_present_bit(entry_addr))
        {
            //kernel_printf("[%02d] BE  %02X %02X %02X %02X %02X %02X %02X %02X \n", i, entry_addr[0], entry_addr[1], entry_addr[2], entry_addr[3],
            //                                                                        entry_addr[4], entry_addr[5], entry_addr[6], entry_addr[7]);

            //kernel_printf("[%02d] LE  %02X %02X %02X %02X %02X %02X %02X %02X \n", i, entry_addr[7], entry_addr[6], entry_addr[5], entry_addr[4],
            //                                                                        entry_addr[3], entry_addr[2], entry_addr[1], entry_addr[0]);
            kernel_printf("[%02x] => \n", i);

            kernel_printf("     Present bit      = 0x%X \n", get_descriptor_present_bit(entry_addr));
            kernel_printf("     Offset address   = 0x%X \n", get_descriptor_offset(entry_addr));
            kernel_printf("     Segment selector = 0x%X / %d \n", get_descriptor_segment(entry_addr), get_descriptor_segment(entry_addr));
            kernel_printf("     Gate type        = 0x%X \n", get_descriptor_gate_type(entry_addr));
            kernel_printf("     Privilege level  = 0x%X \n", get_descriptor_privilege(entry_addr));
            kernel_printf("\n");
        }
    }
#endif
}


///////////////////////////////////////////////////////////// Local functions /////////////////////////////////////////////////////////////

static void add_IDT_entry(idt_hdr_t* idt_hdr, uint8_t vector_id, uint8_t privilage, uint8_t gate_type, uint16_t segment, void* func_handler)
{
        uint32_t int_handler = (uint32_t) func_handler;

        kernel_printf("IDT Interrupt table: Registering vector no. %d with handler at 0x%x \n", vector_id, int_handler);

        uint8_t* entry_addr = get_IDT_table_entry(idt_hdr, vector_id);

        set_descriptor_present_bit(entry_addr, 1);
        set_descriptor_privilege(entry_addr, privilage);
        set_descriptor_gate_type(entry_addr, gate_type);  // TODO: Added define for gate types
        set_descriptor_segment(entry_addr, segment); // TODO: Get segment selector from GDT code instead of hardcoding
        set_descriptor_offset(entry_addr, int_handler);
}

static uint32_t get_descriptor_offset(uint8_t* entry_addr)
{
    //return 0;

    uint32_t offset = 0;

    // TODO: Add names to hardcoded indexe numbers
    uint8_t offset_bit_00_07 = entry_addr[0];
    uint8_t offset_bit_08_15 = entry_addr[1];
    uint8_t offset_bit_16_23 = entry_addr[6];
    uint8_t offset_bit_24_31 = entry_addr[7];

    offset += offset_bit_00_07;
    offset += offset_bit_08_15 << 8;
    offset += offset_bit_16_23 << 16;
    offset += offset_bit_24_31 << 24;

    return offset;
}

static void set_descriptor_offset(uint8_t* entry_addr, uint32_t offset_addr)
{
    entry_addr[0] = (offset_addr >>  0) & 0xff;
    entry_addr[1] = (offset_addr >>  8) & 0xff;
    entry_addr[6] = (offset_addr >> 16) & 0xff;
    entry_addr[7] = (offset_addr >> 24) & 0xff;
}

static uint16_t get_descriptor_segment(uint8_t* entry_addr)
{
    uint16_t segement_selector = 0;

    uint8_t segement_selector_00_07 = entry_addr[2];
    uint8_t segement_selector_07_15 = entry_addr[3];

    segement_selector += segement_selector_00_07;
    segement_selector += segement_selector_07_15 << 8;

    return segement_selector;
}

static void set_descriptor_segment(uint8_t* entry_addr, uint16_t segement_selector)
{
    entry_addr[2] = (segement_selector >> 0) & 0xff;
    entry_addr[3] = (segement_selector >> 8) & 0xff;
}

static uint8_t get_descriptor_gate_type(uint8_t* entry_addr)
{
    return entry_addr[5] & 0xf;
}

static void set_descriptor_gate_type(uint8_t* entry_addr, uint8_t gate_type)
{
    if ((gate_type < 0x5 || gate_type > 0x7) && (gate_type != 0xE && gate_type != 0xF))
    {
        kernel_printf("\n[ERROR] IDT set_descriptor_privilege(): Invalid gate_type! gate_type = %d \n", gate_type);
        gate_type = 0;
    }

    entry_addr[5] |= gate_type & 0xf;
}

static uint32_t get_descriptor_privilege(uint8_t* entry_addr)
{
    return (entry_addr[5] >> 5) & 0x3;
}

static void set_descriptor_privilege(uint8_t* entry_addr, uint8_t CPU_privilege)
{
    if (CPU_privilege > 3)
    {
        kernel_printf("\n[ERROR] IDT set_descriptor_privilege(): Invalid privilege number! CPU_privilege = %d \n", CPU_privilege);
        CPU_privilege = 3;
    }

    entry_addr[5] |= (CPU_privilege & 0x3) << 5;
}

static uint8_t get_descriptor_present_bit(uint8_t* entry_addr)
{
    return entry_addr[5] >> 7;
}

static void set_descriptor_present_bit(uint8_t* entry_addr, uint8_t value)
{
    entry_addr[5] |= value << 7;
}

static uint8_t* get_IDT_table_entry(idt_hdr_t* idt_hdr, uint8_t index)
{
    return (uint8_t*) idt_hdr->base + index * IDT_ENTRY_SIZE;
}

/*
    Interrupt instructions:

        INT
        INT3    (0xCC)  - Triggers entry no. 3 in IDT (type "Trap")
        INTO    (0xCE)  - Triggers entry no. 4 in IDT (type "Trap")

    IDS table items:

        Int no.         Type        Name                    Source
        Hex     Dec
        0x00	0	    Fault	    No	Divide Error        DIV and IDIV instructions.
        0x01	1	    Trap	    No	Debug Exception     Instruction, data, and I/O breakpoints; single-step; and others.
        0x02	2	    Interrupt	No	NMI Interrupt       Nonmaskable external interrupt.
        0x03	3	    Trap	    No	Breakpoint          INT3 instruction.
        0x04	4	    Trap	    No	Overflow            INTO instruction.

        0x20 ... 0xff   Interrupt   N/A                     External interrupts.

        0x16 ... 0x1f   RESERVED


*/

/*

How the BIOS or GRUB's GDT table looks like

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
