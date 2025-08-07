# almostOS
Something almost but not quite entirely unlike OS. Writing an OS as I go, for my own amusement and fun (some would say I'm a very boring person). Therefore, there are no big upfront plans for architecture; everything is done ad-hoc and improved iteratively later.

**Current state:** At the moment, it's basically just a bootable shell-like program (as there is no resource management yet), with a very basic custom filesystem on a RAM disk, supporting simple versions of 'ls' and 'cat' commands. It also has a command "dump memory", similar to the hexdump, but dumping the content of memory instead of a file.

It's at a very early stage. It boots, but currently, it can only output text to the screen and read input from the keyboard. I'm not sure in which direction I will go with this project, but I will definitely at least implement an ATA driver to have a filesystem on the actual disk instead of an in-memory one. Had only tested it on QEMU and Bosch x86 PC emulator, but in theory, it should also boot on VirtualBox/VMWare.

Currently, I'm implementing a very simple file system named TinyFS. The file system at the moment has only the most basic functionality, but it is mature enough to be mounted on Linux (and on AlmostOS)

#### It is ugly, but not ugly enough to be rejected by GRUB
![almostOS](/almostOS.png?raw=true "almostOS")

## How to run it

### Prerequisites
apt install nasm build-essential genisoimage qemu-system

### Running the OS
Run:

`make qemu`

This command will compile everything, create an ISO image containing the kernel and Grub, and then run QEMU with this image loaded

### Available commands in the OS

`help`

`dump address`  (dumps the 128 bytes of the memory starting from the specified memory address)

`ls` (list all files)

`cat filename` (outputs the content of the file)

(Yep, that's everything)

## Entry point

The kernel entry point is in the file "kernel_startup.asm", which sets the Multiboot signature and defines the function start_kernel(). This function is defined in the linker script "kernel.ld" as the entry point for the compiled artifact. The start_kernel() just outputs the string "hello_msg", sets the stack pointer, and then jumps to the first C code function: kernel_c_main().

## Statistics

```
Lines of code:      4679
Number of files:      60
OS market share:       1 (user)
```
