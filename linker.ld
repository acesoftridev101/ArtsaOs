<CodeBlock language="text" editable>
ENTRY(_start)

SECTIONS
{
. = 1M;

.text ALIGN(4K) :
{
KEEP(*(.multiboot))
*(.text.boot)
(.text .text.)
}

.rodata ALIGN(4K) : { (.rodata .rodata.) }
.data ALIGN(4K) : { (.data .data.) }

.bss ALIGN(4K) :
{
*(COMMON)
(.bss .bss.)
}
}
</CodeBlock>

Compile and link:


bash
i686-elf-g++ -std=gnu++17 -m32 -O2 \
  -ffreestanding -fno-exceptions -fno-rtti \
  -fno-stack-protector -fno-pic -fno-pie \
  -fno-threadsafe-statics -fno-use-cxa-atexit \
  -fno-builtin -c artsaos.cpp -o artsaos.o

i686-elf-g++ -m32 -T linker.ld \
  -ffreestanding -nostdlib -no-pie \
  -Wl,--build-id=none \
  -o artsaos.elf artsaos.o -lgcc

grub-file --is-x86-multiboot artsaos.elf
