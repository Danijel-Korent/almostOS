
// This needs to be compiled with -nostdlib since there is not standard lib available yet


void kernel_c_main( void )
{
	unsigned char *VGA_RAM = 0x000B8000 + 320;

	for (int i = 0; i < 1000;)
	{
		VGA_RAM[i++] = 'A';
		VGA_RAM[i++] = 0x13;
	}
}
