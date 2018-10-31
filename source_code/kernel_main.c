
// This needs to be compiled with -nostdlib since there is not standard lib available yet

static int test_var1 = 0xaa;
static int test_var2;
static int test_var3;

int kernel_c_main( void )
{
	static int test_result;
	
	test_result = test_var1 + test_var2 + test_var3;
	
	return test_result;
}
