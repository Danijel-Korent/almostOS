
/*
 *  INFO:
 *
 *      - https://www.math.utah.edu/docs/info/ld_3.html
 *      - https://sourceware.org/binutils/docs-2.25/ld/Scripts.html#Scripts
 */

ENTRY(start_kernel)

SECTIONS
{
    /*
     *   "." is special linker variable containing the current output location counter.
     *   Set it here to 1MB to skip BIOS address space and all of the lower part of memory-mapped IO on a IBM PC platform
     */
    . = 0x00100000; 


    .text :
    {
        *(.text)
        *(.rotext)
        *(.rodata)
        _end_of_text_seg = ABSOLUTE(.);
    }

    .data :
    {
        *(.data)
        _end_of_data_seg = ABSOLUTE(.);
    }

    .bss :
    {
        *(.bss)
        _end_of_bss_seg = ABSOLUTE(.);
    }
}


