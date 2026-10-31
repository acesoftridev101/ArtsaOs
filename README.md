AstraOS



AstraOS is an experimental 32-bit x86 operating system written in C++.



Features

Multiboot 1 boot support

GRUB bootloader

VGA text-mode console

PS/2 keyboard input

Interactive command interpreter

Basic system information

Reboot and CPU halt commands

Architecture

Language: C++

Architecture: x86 32-bit

Bootloader: GRUB

Boot protocol: Multiboot 1

Status



AstraOS is currently an experimental hobby operating system under development.



Roadmap

Memory management

Interrupt handling

Graphical desktop

Mouse support

Filesystem

Application support



Warning: This project is experimental. Test it in an emulator such as QEMU before attempting to boot physical hardware.

Also! Me (so as acesoftridev101) will NOT be responsible for any of the damages that is caused with improper use.

This took me about a year to make. Don't mind the file size, but the file size is really small. I did not vibe code it. This needs GRUB and QEMU I think so that it can work.

I never tested this operating system before, so don't expect it to work all the time. This operating system is made in Makefile, Shell, Assembly, Linker Script, and c++.

By the way, this is my first operating system I ever made. I needed a lot of experience in programming to make this operating system. 

You will need to compile these since it's just raw source code right now that isn't compiled. That means it cannot run. You compile it into an .exe file then run it. Not on the Makefile, but on the source code of Astraos.cpp you compile it. You run the Makefile first though. Keep the Makefile, bendo.bash, boot.s, grub.cfg, and the linker.ld with the Astraos.cpp, the .gitignore, and the readme.md.
