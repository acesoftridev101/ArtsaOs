/*
    AstraOS v0.1
    Single-file freestanding C++ x86 kernel
    Boot protocol: Multiboot 1
    Architecture: 32-bit x86

    Features:
      - Embedded assembly boot entry
      - VGA text console
      - PS/2 keyboard polling
      - Command interpreter
      - Desktop-style welcome screen
      - Basic system information
*/

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned int   size_t;

static inline u8 inb(u16 port) {
    u8 result;
asm volatile ("inb %1, %0"
        : "=a"(result) : "Nd"(port));
return result;
}

static inline void outb(u16 port, u8 value)
{
    asm volatile ("outb %0, %1"
        : : "a"(value), "Nd"(port));
}

static inline void halt_cpu()
{
    asm volatile ("cli; hlt");
}

// ============================================================
// VGA TEXT DRIVER
// ============================================================

static volatile u16* const VGA =
    (volatile u16*)0xB8000;

static const u32 WIDTH = 80;
static const u32 HEIGHT = 25;

static u32 cx = 0;
static u32 cy = 0;
static u8 color = 0x0F;

static u16 cell(char c, u8 attr)
{
    return (u16)(u8)c | ((u16)attr << 8);
}

static void clear_screen()
{
    for (u32 i = 0; i < WIDTH * HEIGHT; ++i)
        VGA[i] = cell(' ', color);

    cx = 0;
    cy = 0;
}

static void scroll()
{
    if (cy < HEIGHT) return;

    for (u32 y = 1; y < HEIGHT; ++y)
        for (u32 x = 0; x < WIDTH; ++x)
            VGA[(y - 1) * WIDTH + x] =
                VGA[y * WIDTH + x];

    for (u32 x = 0; x < WIDTH; ++x)
        VGA[(HEIGHT - 1) * WIDTH + x] =
            cell(' ', color);

    cy = HEIGHT - 1;
}

static void putc(char c)
{
    if (c == '\n')
    {
        cx = 0;
        ++cy;
    }
    else if (c == '\r')
    {
        cx = 0;
    }
    else if (c == '\b')
    {
        if (cx)
        {
            --cx;
            VGA[cy * WIDTH + cx] = cell(' ', color);
        }
        return;
    }
    else if (c == '\t')
    {
        cx = (cx + 4) & ~3u;
    }
    else
    {
        VGA[cy * WIDTH + cx] = cell(c, color);

        if (++cx >= WIDTH)
        {
            cx = 0;
            ++cy;
        }
    }

    scroll();
}

static void print(const char* s)
{
    while (*s) putc(*s++);
}

static void println(const char* s)
{
    print(s);
    putc('\n');
}

static void setcolor(u8 c)
{
    color = c;
}

static bool equals(const char* a, const char* b)
{
    while (*a && *b && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

// ============================================================
// KEYBOARD DRIVER: BASIC SET-1 SCANCODES
// ============================================================

static const char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0',
    '-','=','\b','\t',
    'q','w','e','r','t','y','u','i','o','p','[',']',
    '\n',0,'a','s','d','f','g','h','j','k','l',';','\'',
    '`',0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,'*',0,' '
};

static bool shift = false;
static bool caps = false;

static char keyboard_read()
{
    if (!(inb(0x64) & 1))
        return 0;

    u8 sc = inb(0x60);

    if (sc & 0x80)
    {
        u8 released = sc & 0x7F;
        if (released == 42 || released == 54)
            shift = false;
        return 0;
    }

    if (sc == 42 || sc == 54)
    {
        shift = true;
        return 0;
    }

    if (sc == 58)
    {
        caps = !caps;
        return 0;
    }

    if (sc >= 128)
        return 0;

    char c = keymap[sc];

    if (c >= 'a' && c <= 'z')
    {
        if (shift != caps)
            c -= 'a' - 'A';
    }
    else if (shift)
    {
        switch (c)
        {
            case '1': c = '!'; break;
            case '2': c = '@'; break;
            case '3': c = '#'; break;
            case '4': c = '$'; break;
            case '5': c = '%'; break;
            case '6': c = '^'; break;
            case '7': c = '&'; break;
            case '8': c = '*'; break;
            case '9': c = '('; break;
            case '0': c = ')'; break;
            case '-': c = '_'; break;
            case '=': c = '+'; break;
        }
    }

    return c;
}

static char getkey()
{
    char c;
    while (!(c = keyboard_read()))
    {
        // Poll hardware until a key arrives.
    }
    return c;
}

// ============================================================
// ASTRAOS COMMAND SHELL
// ============================================================

static char input[128];

static void prompt()
{
    setcolor(0x0B);
    print("astra");
    setcolor(0x07);
    print("@system");
    setcolor(0x0F);
    print(":~$ ");
}

static void help()
{
    println("ASTRAOS COMMANDS");
    println("------------------------------");
    println("help      - Show commands");
    println("about     - About AstraOS");
    println("clear     - Clear the screen");
    println("version   - Kernel version");
    println("mem       - Memory status");
    println("apps      - Installed commands");
    println("desktop   - Return to welcome screen");
    println("reboot    - Request system reset");
    println("halt      - Halt the CPU");
    println("------------------------------");
}

static void desktop()
{
    clear_screen();

    setcolor(0x09);
    println("================================================");
    println("                 A S T R A O S                  ");
    println("================================================");

    setcolor(0x0F);
    println("");
    println("  Welcome to AstraOS.");
    println("  A freestanding C++ operating system project.");
    println("");
    println("  [1] Terminal");
    println("  [2] System information");
    println("  [3] Help");
    println("");
    println("  Type a command below to get started.");
    println("");
}

static void reboot()
{
    asm volatile ("cli");

    for (u32 i = 0; i < 100000; ++i)
        if (!(inb(0x64) & 2))
            break;

    outb(0x64, 0xFE);

    for (; ; )
        halt_cpu();
}

static void run_command(char* cmd)
{
    if (equals(cmd, "help"))
    {
        help();
    }
    else if (equals(cmd, "about"))
    {
        println("AstraOS: experimental freestanding C++ kernel.");
        println("Architecture: x86 32-bit.");
        println("Boot protocol: Multiboot 1.");
    }
    else if (equals(cmd, "version"))
    {
        println("AstraOS v0.1.0");
    }
    else if (equals(cmd, "clear"))
    {
        clear_screen();
    }
    else if (equals(cmd, "mem"))
    {
        println("Memory manager: not implemented yet.");
        println("Heap: not initialized.");
        println("Paging: not initialized.");
    }
    else if (equals(cmd, "apps"))
    {
        println("Built-in applications:");
        println("  terminal - command interpreter");
        println("  desktop  - welcome screen");
        println("  help     - command reference");
    }
    else if (equals(cmd, "desktop"))
    {
        desktop();
    }
    else if (equals(cmd, "reboot"))
    {
        println("Requesting hardware reset...");
        reboot();
    }
    else if (equals(cmd, "halt"))
    {
        println("CPU halted. Reset the virtual machine to continue.");
        for (; ; )
            halt_cpu();
    }
    else if (cmd[0])
    {
        print("Unknown command: ");
        println(cmd);
        println("Type help for available commands.");
    }
}

// ============================================================
// KERNEL MAIN
// ============================================================

extern "C" void kernel_main(u32 multiboot_magic, u32 multiboot_info)
{
    (void)multiboot_info;

    asm volatile ("cli");

    color = 0x0F;
    desktop();

    if (multiboot_magic != 0x2BADB002)
    {
        println("ERROR: Invalid Multiboot boot signature.");
        for (; ; )
            halt_cpu();
    }

    for (; ; )
    {
        prompt();

        u32 length = 0;

        for (; ; )
        {
            char c = getkey();

            if (c == '\n')
            {
                putc('\n');
                input[length] = '\0';
                break;
            }

            if (c == '\b')
            {
                if (length)
                {
                    --length;
                    putc('\b');
                }
                continue;
            }

            if (c >= 32 && c <= 126 && length < sizeof(input) - 1)
            {
                input[length++] = c;
                putc(c);
            }
        }

        run_command(input);
    }
}

