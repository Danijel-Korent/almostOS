# almostOS
Something almost but not quite entirely unlike OS. Implementing it for my own amusement and fun (some would say I'm a very boring person). There are no big upfront plans for architecture; everything is done ad-hoc and improved iteratively later.

**Current state:**
It's at a very early stage. At the moment, it looks more lika a bootable shell-like program than an OS, with a very basic custom filesystem on a RAM disk, supporting simple versions of 'ls' and 'cat' commands. It also has a command "dump memory", similar to the hexdump, but dumping the content of memory instead of a file.

What it has or can do:
- A rudimentary scheduler
- A simple FAT-like filesystem
  - at the moment it has only the most basic functionality, but it is mature enough to be mounted on Linux (and on AlmostOS)
- Can output text to VGA or COM port
- Can read input from keyboard or COM port
- Support for "write" and "exit" system calls
  - System calls are Linux compatible, meaning, it can now execute (artificialy) simple Linux 32-bit x86 programs
- rudimentary shell
  - The shell is built into the OS, as currently there are still not enough system calls available to implement a shell in userspace


I'm not sure in which direction I will go with this project, but I will definitely at least implement an ATA driver to have a filesystem on the actual disk instead of an in-memory one. Had only tested it on QEMU and Bosch x86 PC emulator, but in theory, it should also boot on VirtualBox/VMWare.

Currently, I'm implementing my version of "virtual filesystem switch" and adding more system calls


#### It is ugly, but not ugly enough to be rejected by GRUB
![almostOS](/almostOS.png?raw=true "almostOS")

## How to run it

### Prerequisites
apt install nasm build-essential genisoimage qemu-system

### Running the OS
Run:

`make qemu`

This command will compile everything, create an ISO image containing the kernel and Grub, and then run QEMU with this image loaded

### Available commands in the build-in shell

`help`

`ls` (list all files)

`cat filename` (outputs the content of the file)

`ps` (list all running processes)

`kill PID` (kill a process)

`run num` (run a test process)

`dump address`  (dumps the 128 bytes of the memory starting from the specified memory address)

(Yep, that's everything)

## Entry point

The kernel entry point is in the file "kernel_startup.asm", which sets the Multiboot signature and defines the function start_kernel(). This function is defined in the linker script "kernel.ld" as the entry point for the compiled artifact. The start_kernel() just outputs the string "hello_msg", sets the stack pointer, and then jumps to the first C code function: kernel_c_main().

## Statistics

```
Lines of code:      4679
Number of files:      60
OS market share:       1 (user)
```
