CXX = i686-elf-g++
AS  = i686-elf-as

CXXFLAGS = -std=gnu++17 -ffreestanding -O2 -Wall -Wextra \
           -fno-exceptions -fno-rtti -fno-stack-protector \
           -fno-pic -fno-pie -fno-threadsafe-statics \
           -fno-use-cxa-atexit -fno-builtin

all: artsaos.iso

build:
	mkdir -p build/isodir/boot/grub

build/boot.o: boot.s | build
	$(AS) boot.s -o build/boot.o

build/kernel.o: kernel.cpp | build
	$(CXX) $(CXXFLAGS) -c kernel.cpp -o build/kernel.o

build/kernel.elf: build/boot.o build/kernel.o linker.ld
	$(CXX) -T linker.ld -o build/kernel.elf \
		-ffreestanding -nostdlib -no-pie -Wl,--build-id=none \
		build/boot.o build/kernel.o -lgcc

artsaos.iso: build/kernel.elf grub.cfg
	grub-file --is-x86-multiboot build/kernel.elf
	cp build/kernel.elf build/isodir/boot/kernel.elf
	cp grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o artsaos.iso build/isodir

run: artsaos.iso
	qemu-system-i386 -cdrom artsaos.iso

clean:
	rm -rf build artsaos.iso

.PHONY: all clean run
