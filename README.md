# almostOS
Something almost but not quite entirely unlike OS. Writing an OS as I go, for my own amusement and fun (some would say I'm a very boring person). Therefore there are no big upfront plans for architecture, everything is done ad-hoc and improved iteratively later.

**Current state:** It's basically just a bootable shell-like program at the moment (as there is no resource management yet), with a "dump" command similar to hexdump and a very unhelpful "help" command

It's at a very early stage. It boots, but currently it can only output text to the screen and read the input from the keyboard. I'm not sure in which direction I will go with this project, but I will definitely at least implement an ATA driver and some file system parser/driver. Had only tested it on QEMU and Bosch x86 PC emulator, but in theory it should also boot on VirtualBox/VMWare.

#### It's ugly, but not ugly enough to be rejected by GRUB
![almostOS](/almostOS.png?raw=true "almostOS")

## How to run it

### Prerequisites
Install nasm, build-essential, genisoimage and qemu-system

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
