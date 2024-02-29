# almostOS
Something almost but not quite entirely unlike OS. Writing an OS as I go, for my own amusement and fun (some would say I'm a very boring person). Therefore there are no big upfront plans for architecture, everything is done ad-hoc and improved iteratively later.

**Current state:** It's basically just a bootable shell-like program at the moment (as there is no resource management yet), with a command "dump" (similar to hexdump), and a very unhelpful "help" command.

It's at a very early stage. It boots, but currently it can only output text to the screen and read the input from the keyboard. I'm not sure in which direction I will go with this project, but I will definitely at least implement an ATA driver and some file system driver. Had only tested it on QEMU and Bosch x86 PC emulator, but in theory, it should also boot on VirtualBox/VMWare.

Currently, I'm implementing a very simple file system named TinyFS. The file system at the moment has only the most basic functionality but is mature enough to be mounted on Linux. I still have to integrate it into this kernel - unlike in Linux, there is no framework/API for FS integration just yet

#### It is ugly, but not ugly enough to be rejected by GRUB
![almostOS](/almostOS.png?raw=true "almostOS")

## How to run it

### Prerequisites
apt install nasm build-essential genisoimage qemu-system

### Running the OS
Inside "source_code" directory run:

`make qemu`

This command will compile everything, create an ISO image containing the kernel and grub and then run QEMU with the image

### Available commands in the OS

`help`  
`dump 300`  (dumps the 128 bytes of the memory starting from the memory address 0x300)

(Yep, that's everything. Will add more, I promise!)

## Entry point

The kernel entry point is in the file "kernel_startup.asm" which sets Multiboot signature and defines function start_kernel(). This functions is in linker script "kernel.ld" defined as entry point for the compiled artifact. The start_kernel() just outputs string "hello_msg", sets stack pointer and then jumps to the first C code function: kernel_c_main()

## Statistics

```
Lines of code:   1940
Number of files:   28
Number of users:    1
```
