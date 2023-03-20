void LOG(const unsigned char* const message); // TODO
void long_to_hex(long int number, char * string_buffer, int string_buffer_len, unsigned char base);

#define COM1_OFFSET         (0x00)
#define LPT1_OFFSET         (0x08)
#define VIDEO_IO_OFFSET     (0x63)
#define HDD_NO_OFFSET       (0x75)

// TODO: I really need to define stdint types
short int read_16bit( const unsigned char *buffer, int offset)
{
    return (buffer[offset+1] << 8) + buffer[offset];
}


void parse_BIOS_Data_Area(void)
{
    char *BDA_base_address = (char *) 1024;

    LOG("Parsing BIOS Data Area...");

    char com_string[] = "COM1 I/O Port: 0x????";

    short int COM1 = read_16bit(BDA_base_address, COM1_OFFSET);
    long_to_hex(COM1, com_string + 17, 4, 16);

    LOG(com_string);


    char lpt_string[] = "LPT1 I/O Port: 0x????";

    short int LPT1 = read_16bit(BDA_base_address, LPT1_OFFSET);
    long_to_hex(LPT1, lpt_string + 17, 4, 16);

    LOG(lpt_string);


#if 0
    LOG("Video base IO port: ?x????");
    LOG("Hard drives detected: ?x??");

    // Probably will not work since we are in protected mode and BIOS ISRs are 16-bit code and also probably overwriten by bootloader
    LOG("IRQ0 Timer tics: ?x????");

    // Mostly useless since Bootloader could change the mode after BIOS
    LOG("Display mode: ?x??");
    LOG("Text mode columns: ?x????");
#endif
}
