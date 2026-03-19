2026
====

GCC trying to be smart
---------------------------------------------
If I put string literal directly into strncmp() the compiler will replace it with strcmp() I don't have strcmp implemented and am in no mood
to implement it now



21.01. - Fixed int 0x80 system call mechanism
---------------------------------------------

- The issue was when IRQ sched code saved the proces, and then non-IRQ sched code restored the process. The non-IRQ code assumed existend of
  return address on stack which it tried to modify, currupting the stack in process


16.01. - Implemented int 0x80 system call mechanism
---------------------------------------------------

- But it is not working....


2025
====


12.09. - Process switching
--------------------------
- My lazy method of context switching by just saving the stack pointer is now very inconvinient.
- Currently I'm just pushing all regs onto the stack before the switch, and then just swiching stack pointer to the stack of the next
  process and doing normal "ret" instruction (new stack has return address of the next process) - This is now very inconvinient because for
  int 0x80 I need to call "iret" which expects (and pops) more stuff on the stack - I could make an awkward workaround by cleaning the stack
  of current process before doing stack pointer switch, and then prepareing the stack of the next task for "iret" instead of "ret"... but
  the problem is that stack on interrupt is not always the same. Differnt things are pushed, depending if there was a privilege change or
  not. So after implementing privilege levels, I would need to change the workaround again. - The biggest problem is that with this is that
  I need to pop the values from stack first to move them after register values, but I cannot pop then into registers as I need to preserve
  them first
- So I would need to move this values without "pop" instruction, which requires mov [mem], [mem], ideally both relative to the stack pointer
  - if any "mov" instruction even support relative addresing on both sides
- anyway, just too much of awkawrd and hard to read code to be worth it 
- I guess it is time to implement normal process switching. I will need to manipulate stack in the of "iret" in any case, but in the case of
  "lazy switch" it is a lot more awkawrd with jugling things on stack while also having to preserve register values.



10.09. - x86 Interrupt Descriptor Table
---------------------------------------
- I will only implement software interrupt at 0x80 for system calls, the same location where Linux has its syscall interface, as I want my
  system call interface to be compatible with the Linux one
- I will also add few handlers for exceptions while I'm at it.


14.08. - x86 Memory segmentation and GDT table
----------------------------------------------
- I have added code for printing GDT table data, and have printed the table set up by GRUB
- Unsurprisingly, the table looks exacly as the one I want to set (except for 16-bit mode entries), so I will just re-use it

This is how the GRUBs table looks like::

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

- I have added this data to source file, but who knows when I will decide to delete it.


07.08. - interrupts and system calls
----------------------------------------

- Looks like I cannot avoid handling interrupts anymore, as I need to use software interrupts for doing syscalls
- There are alternatives to software interrupts, as there are specific instructions on x86 for doing syscalls in 32-bit mode (like
  sysenter), but they are not compatible between Intel and AMD cpus in all modes. - For example, when I run instruction "sysenter" in 32-bit
  compatibility mode (64-bit OS running 32-bit app) on AMD, I get "invalid instruction" - Although I do not plan to use "compat" mode in my
  kernel, I want to be able to run all user-space test programs on my 64-bit Linux. - Therefore, software interrupt are my only option. -
  For interrupts we need to set IDT table, but IDT table have references to GDT table, so we need to set them both

More info on the mess with x86 system call instructions in compat mode:
https://reverseengineering.stackexchange.com/questions/16454/struggling-between-syscall-or-sysenter-windows
https://stackoverflow.com/questions/77678700/calling-system-api-from-32-bit-processes-under-linux-64-bit/77680771#77680771


Intro
-----

- Decided to start writing this log as sometimes I'm reading the code and wish I have somewhere documented the context I had back then when
  writing that part
- Or just causually reading the code and ask myself, wtf I did this. Sometimes there is just too much to write into code comments
- Hopefully I will not forgot about the existance of this file
- Anyway, started this project for fun in 2018, made 10 commits that printed message 'Starting kernel...' in a nice green color, then didn't
  touch it for exacly 2 years. 
- After that, every year I would work on it for a few days, when I randomly remembered it exists. 
- But since I've put this project into my CV now, I decided to give it regular dev time and push it to at lest being capable running simple
  Linux-compatible user-space programs
- As currently it's like aplying to SpaceX with CV saying you are working on your very own rocket, but all you really did was put some
  menthos in a Coca Cola bottle and pointed it towards the sky
- Constantly needed to refer something I've forgot about
- Decided to delay the usage of interrupts as much as possible to avoid asychronous code, or more precisely, to avoid asychronous bugs. I
  don't have much free time, definitely not enough of it to spend it on debugging race conditions
