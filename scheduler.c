#include "scheduler.h"

#include "util.h"           // for memset
#include "kernel_stdio.h"   // for kernel_println


///////////////////////////////////// Scheduler test code /////////////////////////////////////

// TODO: Rename to create_process
void init_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void))
{
    kernel_println("Called init_process()");

    memset(new_process, 0, sizeof(struct process_ctx));
    new_process->name = name;

    u8 dummy_var = 0;

    u32 stack_start_addr = 0; 

    stack_start_addr = (u32)&dummy_var - (128 * 1024);         // A hack until I implement proper stack mem management
    stack_start_addr = stack_start_addr & 0xFFFF0000;    // Align the address to 64K boundary for easier reading

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

static struct process_ctx *current_running_process = NULL;
static struct process_ctx kernel_process = {0};
static struct process_ctx test_process = {0};

void test_thread2_handler(void);

void scheduler_init(void)
{
    kernel_println("Called scheduler_init()");
    //kernel_printf("Called scheduler_init() --> %s \n", "TEST");

    kernel_process.name     = "Kernel process";
    current_running_process = &kernel_process;

    // TEMP - test code
    init_process(&test_process, "Test process", test_thread2_handler);
}

void schedule(void)
{
    //kernel_println("Called schedule()");
    struct process_ctx *process_to_restore = NULL;
    struct process_ctx *process_to_save    = current_running_process;

    if (current_running_process == &kernel_process)
    {
        process_to_restore = &test_process;
    }
    else
    {
        process_to_restore = &kernel_process;
    }

    current_running_process = process_to_restore;

    //kernel_printf("schedule(): Current process = \"%s\" \n", process_to_save->name);
    //kernel_printf("schedule(): Next process    = \"%s\" \n", process_to_restore->name);

    // TODO: Assert that process_to_restore != current_running_process
    // TODO: Assert that process_to_restore != process_to_save

    switch_process(process_to_save, process_to_restore);
}


static int test_thread2_counter = 0;

void test_thread2_handler(void)
{
    kernel_println("\n>>>>>>>>>> Start of test_thread2_handler() <<<<<<<<<<< \n");

    while(1)
    {
        u32 counter = 1000 * 1000;

        if (test_thread2_counter % counter == 1)
        {
            kernel_println("!!! TEST THREAD_2 RUNNING !!!");
        }
    
        test_thread2_counter++;
    
        schedule();
    }
}

///////////////////////////////////// /Scheduler test code /////////////////////////////////////

