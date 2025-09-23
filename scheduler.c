#include "scheduler.h"

#include "util.h"           // for memset
#include "kernel_stdio.h"   // for kernel_println


static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void));

static void test_thread2_handler(void);
static void test_thread3_handler(void);
static void test_thread4_handler(void);

// This array holds info about all processes in the system
// For now, process_ctx.name == NULL means that slot is not used
static struct process_ctx process_list[16] = {0}; // 16 processes ought to be enough for everyone (I need to stop making this jokes)

static int current_process_index = 0;



void scheduler_init(void)
{
    kernel_println("\nCalled scheduler_init()");

    // No need to set anything execept name as other struct members will be set on the first process switch
    current_process_index = 0;
    process_list[current_process_index].name = "Kernel process";

    create_process(&process_list[1], "Test process 2", test_thread2_handler);
    create_process(&process_list[4], "Test process 3", test_thread3_handler);
    create_process(&process_list[15], "Test process 4", test_thread4_handler);
}

void schedule(void)
{
    //kernel_println("Called schedule()");
    struct process_ctx *process_to_restore = NULL;
    struct process_ctx *process_to_save    = &process_list[current_process_index];

    int i = current_process_index;

    while(1)
    {
        i++;
        i = i % (sizeof(process_list) / sizeof(process_list[0]));

        if (process_list[i].name == NULL)
        {
            continue;
        }
        else
        {
            process_to_restore = &process_list[i];
            current_process_index = i;
            break;
        }
    }

    if (process_to_restore == NULL)
    {
        kernel_println("\n [ERROR] process_to_restore == NULL");
        while(1);
    }

    //kernel_printf("schedule(): Current process = \"%s\" \n", process_to_save->name);
    //kernel_printf("schedule(): Next process    = \"%s\" \n", process_to_restore->name);

    if (process_to_save != process_to_restore)
    {
        switch_process(process_to_save, process_to_restore);
    }
}


// This is really just a pointer that grows in one direction, but never shrinks, so it cannot deallocate space
// Good enough for initial testing, but we cannot dinamically create and delete processes because the pointer
// would quickly run out of memory
// TODO:
//      1) Instead of pointer growing in one direction, split stack area into 64k chunks and keep trace of free was used
//          - Isn't this just re-implementation of current heap_malloc() ??
//
//      2) Use heap_malloc()
//          - For that I need to add support for multiple bin sizes (I need to do that anyway)
//              or
//          - much faster solution would be to make allocator instance based, and just make new instace with PROCESS_STACK_SIZE bins
//
static int poors_man_stack_allocator = 0;


// TODO: There is some x86 specific code that needs to be moved to arch/x86-32
static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void))
{
    kernel_println("Called init_process()");

    memset(new_process, 0, sizeof(struct process_ctx));
    new_process->name = name;

    u8 dummy_var = 0;

    u32 stack_start_addr = 0;

    #define PROCESS_STACK_SIZE (128 * 1024)

    if (poors_man_stack_allocator == 0)
    {
        poors_man_stack_allocator = (u32)&dummy_var - PROCESS_STACK_SIZE;       // TODO: A hack until I implement proper stack memomory management
        poors_man_stack_allocator = poors_man_stack_allocator & 0xFFFF0000;     // Align the address to 64K boundary for easier reading/debugging
    }
    else
    {
        poors_man_stack_allocator -= PROCESS_STACK_SIZE;
    }

    if (poors_man_stack_allocator <= PROCESS_STACK_SIZE)
    {
        kernel_printf("[ERROR] create_process(): No more stack space! poors_man_stack_allocator = 0x%x", poors_man_stack_allocator);
        return;
    }

    stack_start_addr = (u32)poors_man_stack_allocator;

    u32 *process_stack = (u32*) stack_start_addr;

    // TODO: Specific to x86 cdecl, therefore must be moved to x86 specific code
    //       More directly, this is related to the instructions call/ret, whose usage "cdecl" asumes
    // TODO2: Remove this and only use 
    process_stack[0]  = (u32) func_ptr;     // Return address

    // This whole struct is x86-specific, so after adding RISC-V support, this part will need to be moved to arch/x86
    // and here only use an opaque pointer containing architecture specific stuff
    new_process->reg_esp = (u32) process_stack;
    new_process->reg_ip  = (u32) func_ptr;
    new_process->reg_eflags = 0x00000002; // EFLAGS // TODO: This value disables interupts, but that is not important at the moment

    kernel_printf("init_process(): Created process \"%s\" \n", name);
    kernel_printf("init_process():   entry = 0x%x \n", func_ptr);
    kernel_printf("init_process():   stack = 0x%x \n", stack_start_addr);
    kernel_printf("init_process():   SP    = 0x%x \n", new_process->reg_esp);
}


///////////////////////////////////// Scheduler test code /////////////////////////////////////

static void thread_test(const char* func_name, u32 counter_target)
{
    kernel_printf("\n[%s] Start of %s() \n", func_name, func_name);

    int test_thread2_counter = 0;

    while(1)
    {
        //u32 counter = 1000 * 1000;

        if (test_thread2_counter % counter_target == 1)
        {
            kernel_printf("\n[%s] Still running... ", func_name);
            //sys_write(1, "TEST \n", 7);
        }
    
        test_thread2_counter++;
    
        schedule();
    }
}

static void test_thread2_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (1000*1000));
}

static void test_thread3_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (2*1000*1000));
}

static void test_thread4_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (2*1000*1000));
}


///////////////////////////////////// /Scheduler test code /////////////////////////////////////

