#include "scheduler.h"

#include "util.h"           // for memset
#include "kernel_stdio.h"   // for kernel_println


///////////////////////////////////// Scheduler test code /////////////////////////////////////

static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void));

static void test_thread2_handler(void);
static void test_thread3_handler(void);
static void test_thread4_handler(void);

static struct process_ctx process_list[16] = {0}; // 16 processes ought to be enough for everyone (I need to stop making this jokes)

static int process_list_index = 0;

static struct process_ctx *current_running_process = NULL;
static struct process_ctx kernel_process = {0};
static struct process_ctx test_process = {0};


void scheduler_init(void)
{
    kernel_println("Called scheduler_init()");
    //kernel_printf("Called scheduler_init() --> %s \n", "TEST");

#if 0
    kernel_process.name     = "Kernel process";
    current_running_process = &kernel_process;

    // TEMP - test code
    create_process(&test_process, "Test process", test_thread2_handler);
#else
    kernel_process.name     = "Kernel process";
    process_list[0] = kernel_process;
    //mem_copy(&process_list[0], sizeof(process_list[0]), kernel_process, sizeof(struct process_ctx));

    current_running_process = &process_list[0];

    //create_process(&process_list[1], "Test process 2", test_thread2_handler);
    //create_process(&process_list[4], "Test process 3", test_thread3_handler);
    create_process(&process_list[15], "Test process 4", test_thread4_handler);
#endif
}

void schedule(void)
{
    //kernel_println("Called schedule()");
    struct process_ctx *process_to_restore = NULL;
    struct process_ctx *process_to_save    = current_running_process;

#if 0
    if (current_running_process == &kernel_process)
    {
        process_to_restore = &test_process;
    }
    else
    {
        process_to_restore = &kernel_process;
    }
#else
    process_to_restore = current_running_process;

    int i = process_list_index;

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
            process_list_index = i;
            break;
        }
    }
#endif

    current_running_process = process_to_restore;

    //kernel_printf("schedule(): Current process = \"%s\" \n", process_to_save->name);
    //kernel_printf("schedule(): Next process    = \"%s\" \n", process_to_restore->name);

    // TODO: Assert that process_to_restore != current_running_process
    // TODO: Assert that process_to_restore != process_to_save

    switch_process(process_to_save, process_to_restore);
}


// This is really just a pointer that grows in one direction, but never shrinks, so it cannot deallocate space
// Good enough for initial testing, but we cannot dinamically create and delete processes because the pointer
// would quickly run out of memory
// TODO:
//      - Use heap_malloc()
//          - For that I need to add support for multiple bin sizes (I need to do that anyway)
//              or
//          - much faster solution would be to make allocator instance based, and just make new instace with PROCESS_STACK_SIZE bins
static int poors_man_stack_allocator = 0;


// TODO: This is x86 specific code, move to arch/x86-32
static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void))
{
    kernel_println("Called init_process()");

    memset(new_process, 0, sizeof(struct process_ctx));
    new_process->name = name;

    u8 dummy_var = 0;

    u32 stack_start_addr = 0;

#if 0
    stack_start_addr = (u32)&dummy_var - (128 * 1024);  // TODO: A hack until I implement proper stack mem management
    stack_start_addr = stack_start_addr & 0xFFFF0000;   // Align the address to 64K boundary for easier reading
#else

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
        kernel_printf("[ERROR] create_process(): No more stack space! poors_man_stack_allocator = %x", poors_man_stack_allocator);
        return;
    }

    stack_start_addr = (u32)poors_man_stack_allocator;
#endif


    u32 *process_stack = (u32*) stack_start_addr;

    // TODO: Specific to x86 cdecl, therefore must be moved to x86 specific code
    process_stack[0]  = (u32) func_ptr;     // Return address
    // process_stack[-1] = stack_start_addr;    // Saved BP - TODO: I should probably put jmp panic to address 0 while developing this
                                                // Update:   I thought EBP is saved here, but it is not. All this blog posts cannot be wrong?
                                                // Updated2: Forgot that in the cdelc convention BP is pushed by callee, not by caller
                                                //           So usually it is there after function is called, but in my switch_process()
                                                //           I don't do that as this func doesn't call any C code and doesn't have any local vars

    // Pushed by pushad, x86 specific
    process_stack[-1] = 0;   // edi
    process_stack[-2] = 0;   // esi
    process_stack[-3] = 0;   // ebp
    process_stack[-4] = 0;   // esp
    process_stack[-5] = 0;   // ebx
    process_stack[-6] = 0;   // edx
    process_stack[-7] = 0;   // ecx
    process_stack[-8] = 0;   // eax
    // Pushed by pushfd
    process_stack[-9] = 0x00000002; // EFLAGS // TODO: This value disables interupts, but that is not important at the moment

    new_process->reg_esp = (u32) &process_stack[-9];

    kernel_printf("init_process(): Created process \"%s\" \n", name);
    kernel_printf("init_process():   entry = %x \n", func_ptr);
    kernel_printf("init_process():   stack = %x \n", stack_start_addr);
    kernel_printf("init_process():   SP    = %x \n", new_process->reg_esp);
}

static void thread_test(const char* func_name, u32 counter_target)
{
    kernel_printf("\n>>>>>>>>>> Start of %s() <<<<<<<<<<< \n", func_name);

    int test_thread2_counter = 0;

    while(1)
    {
        //u32 counter = 1000 * 1000;

        if (test_thread2_counter % counter_target == 1)
        {
            kernel_printf("[%s] Still running... ", func_name);
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
    thread_test(__PRETTY_FUNCTION__, (4*1000*1000));
}


///////////////////////////////////// /Scheduler test code /////////////////////////////////////

