#include "scheduler.h"

#include "panic.h"
#include "util.h"           // for memset
#include "kernel_stdio.h"   // for kernel_println
#include "stack.h"

static bool init_process_stack_allocator(void);
static u8* allocate_stack_memory(void);
static void free_stack_memory(u8* ptr);

static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void));

static void test_thread_2_handler(void);
static void test_thread_3_handler(void);
static void test_thread_4_handler(void);

// This array holds info about all processes in the system
// For now, process_ctx.name == NULL means that slot is not used
static struct process_ctx process_list[16] = {0}; // 16 processes ought to be enough for everyone (I need to stop making this jokes)

static int current_process_index = 0;

void syscall_test(void);

void scheduler_init(void)
{
    kernel_println("\nCalled scheduler_init()");

    // No need to set anything execept name as other struct members will be set on the first process switch
    current_process_index = 0;
    process_list[current_process_index].name = "Kernel process";

    if (init_process_stack_allocator())
    {
        create_process(&process_list[1], "Test process 2", test_thread_2_handler);
        create_process(&process_list[4], "Test process 3", test_thread_3_handler);
        //create_process(&process_list[15], "Test process 4", test_thread_4_handler);
        create_process(&process_list[15], "syscall_test", syscall_test);
    }
    else
    {
        kernel_println("\nscheduler_init(): Failed to init init_process_stack_allocator()");
        time_to_die();
    }
}

struct process_ctx * get_current_process(void)
{
    return &process_list[current_process_index];
}

// Simple round-robin
struct process_ctx* move_to_next_process(void)
{
    int i = current_process_index;

    while(1)
    {
        i++;

        // To prevent going outside of array bounds
        i = i % (sizeof(process_list) / sizeof(process_list[0]));

        if (process_list[i].name == NULL) // This means the slot is unused // TODO: Add function for this check
        {
            // Skip unused slots
            continue;
        }
        else
        {
            // Simply return the first next slot in use
            current_process_index = i;
            return &process_list[i];
        }
    }
}

void schedule(void)
{
    //kernel_println("Called schedule()");
    struct process_ctx *process_to_restore = NULL;
    struct process_ctx *process_to_save    = get_current_process(); //&process_list[current_process_index];

    process_to_restore = move_to_next_process();

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

// Need to implement it in assembly where I have full control of the stack (IRQ saves some values on stack, and reads back on iret)
#if 0
void schedule_in_irq_context(void)
{
    //kernel_println("Called schedule()");
    struct process_ctx *process_to_restore = NULL;
    struct process_ctx *process_to_save    = &process_list[current_process_index];

    process_to_restore = move_to_next_process();

    if (process_to_restore == NULL)
    {
        kernel_println("\n [ERROR] process_to_restore == NULL");
        while(1);
    }

    //kernel_printf("schedule(): Current process = \"%s\" \n", process_to_save->name);
    //kernel_printf("schedule(): Next process    = \"%s\" \n", process_to_restore->name);

    // switch_process_in_irq() must be always called to execute "iret" in order to exit the interrupt handler
    switch_process_in_irq(process_to_save, process_to_restore);
}
#endif

// TODO: Better name for this function at the moment would be init_process_ctx, but maybe later it will begin to look like create_process()
// TODO: There is some x86 specific code that needs to be moved to arch/x86-32
static void create_process(struct process_ctx *new_process, const char* name, void (*func_ptr)(void))
{
    kernel_println("Called init_process()");

    memset(new_process, 0, sizeof(struct process_ctx));
    new_process->name = name;

    // This whole struct is x86-specific, so after adding RISC-V support, this part will need to be moved to arch/x86
    // and here only use an opaque pointer containing architecture specific stuff
    new_process->reg_eflags = 0x00000002; // EFLAGS // TODO: This value disables interupts, but that is not important at the moment
    new_process->reg_ip  = (u32) func_ptr;

    u8 *stack_mem_base = allocate_stack_memory();

    new_process->stack_mem_base = stack_mem_base;
    new_process->reg_esp  = (u32) stack_mem_base;

    kernel_printf("init_process(): Created process \"%s\" \n", name);
    kernel_printf("init_process():   entry = 0x%x \n", func_ptr);
    kernel_printf("init_process():   stack = 0x%x \n", new_process->reg_esp);
}

static void kill_process(int process_index)
{
    process_list[process_index].name = NULL; // This means the slot is unused // TODO: Add function for this check

    free_stack_memory(process_list[process_index].stack_mem_base);
}

void kill_current_process(void)
{
    if (current_process_index != 0) // Do not kill kernel thread
    {
        kill_process(current_process_index);
    }
}


///////////////////////////////////// Stack memory management /////////////////////////////////////

// Still not a great solution, not fully dynamic, but better than the previous "sliding pointer" implementation which could only move in one direction
// Now I can now free the allocated memory, which gives me ability to freely kill and create processes

#define PROCESS_STACK_SIZE  (128 * 1024) // This must always be to the power of 2, otherwise code will break in some places (like on aligment)
#define PROCESS_STACK_COUNT (32)

static u8 *process_stack_memory_base = NULL;

static u8* stack_idx_to_ptr(u8 idx)
{
    if (idx >= PROCESS_STACK_COUNT)
    {
        // TODO: Log an error
        return NULL;
    }

    return process_stack_memory_base + (idx * PROCESS_STACK_SIZE);
}

static u8 stack_ptr_to_idx(u8 *ptr)
{
    if (ptr >= (process_stack_memory_base + PROCESS_STACK_SIZE*PROCESS_STACK_COUNT))
    {
        // TODO: Log an error
        return PROCESS_STACK_COUNT;
    }

    return (ptr - process_stack_memory_base) / PROCESS_STACK_SIZE;
}

static stack_handle_t free_blocks_stack;

static u8 process_stack_memory[PROCESS_STACK_SIZE * (PROCESS_STACK_COUNT+1)]; // +1 for aligment

static bool init_process_stack_allocator(void)
{
    kernel_println("\nCalled init_process_stack_allocator()");

    //(unsigned int)();

    process_stack_memory_base = (u8*) (((int)process_stack_memory + PROCESS_STACK_SIZE) & ~(PROCESS_STACK_SIZE-1)); // Align it

    // Setup the stack for free blocks
    {
        static u8 stack_buffer[PROCESS_STACK_COUNT];

        if ( ! stack_init(&free_blocks_stack, stack_buffer, sizeof(stack_buffer)))
        {
            return false;
        }

        // Populate free blocks list
        for (int i = PROCESS_STACK_COUNT-1; i >= 0; i--)
        {
            if ( ! stack_push(&free_blocks_stack, i))
            {
                return false;
            }
        }
    }
}

static u8* allocate_stack_memory(void)
{
    u8 index = 0;
    u8* stack_address = NULL;

    if (stack_pop(&free_blocks_stack, &index))
    {
        if (index >= PROCESS_STACK_COUNT)
        {
            LOG("ERROR: allocate_stack_memory() failed - invalid bin index!");
        }
        else
        {
            stack_address = stack_idx_to_ptr(index);

            LOG("INFO: allocate_stack_memory() - memory successfully allocated");
        }
    }
    else LOG("ERROR: allocate_stack_memory() failed - No free memory!");

    return stack_address;
}

static void free_stack_memory(u8* ptr)
{
    if (ptr == NULL) return; // QTODO: Log an error

    u8 index = stack_ptr_to_idx(ptr);

    if (index >= PROCESS_STACK_COUNT)
    {
        LOG("ERROR: free_stack_memory() failed - wrong index");
    }

    if ( ! stack_push(&free_blocks_stack, index))
    {
        LOG("ERROR: free_stack_memory() failed");
    }
    else
    {
        LOG("INFO: free_stack_memory() - memory successfully freed");
    }
}



///////////////////////////////////// Scheduler test code /////////////////////////////////////

void INT_80_test(void);
void INT_80_test_silent(void);

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
            //INT_80_test();
            INT_80_test_silent();
        }
    
        test_thread2_counter++;
    
        schedule();
    }
}

static void test_thread_2_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (1*1000*1000));
}

static void test_thread_3_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (2*1000*1000));
}

static void test_thread_4_handler(void)
{
    thread_test(__PRETTY_FUNCTION__, (3*1000*1000));
}


///////////////////////////////////// /Scheduler test code /////////////////////////////////////

