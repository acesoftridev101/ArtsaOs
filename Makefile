
CXX = i686-elf-g++
AS = i686-elf-as
LD = i686-elf-ld

CXXFLAGS = -m32 -std=c++17 -ffreestanding \
           -O2 -Wall -Wextra \
           -fno-exceptions -fno-rtti \
           -fno-stack-protector \
           -fno-pie -fno-threadsafe-statics \
           -fno-use-cxa-atexit

all: artsaos.iso

build:
	mkdir -p build/isodir/boot/grub

build/boot.o: boot.s | build
	$(AS) --32 boot.s -o build/boot.o

build/kernel.o: kernel.cpp | build
	$(CXX) $(CXXFLAGS) -c kernel.cpp -o build/kernel.o

build/kernel.elf: build/boot.o build/kernel.o linker.ld
	$(LD) -m elf_i386 -T linker.ld \
		-o build/kernel.elf \
		build/boot.o build/kernel.o -lgcc

artsaos.iso: build/kernel.elf
	grub-file --is-x86-multiboot build/kernel.elf
	cp build/kernel.elf build/isodir/boot/kernel.elf
	cp grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o artsaos.iso build/isodir

clean:
	rm -rf build artsaos.iso

.PHONY: all clean
