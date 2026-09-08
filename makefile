CC = clang
ASM = nasm
LD = ld.lld

CFLAGS = --target=i686-pc-none-elf -march=i686 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -fno-builtin -fno-stack-protector -nostdlib -ffreestanding -mno-red-zone -Iinclude
ASMFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

C_SOURCES = $(wildcard kernel/*.c drivers/*.c fs/*.c mm/*.c modules/*.c)
ASM_SOURCES = $(wildcard kernel/*.asm drivers/*.asm)

C_OBJS = $(patsubst %.c, build/%.o, $(notdir $(C_SOURCES)))
ASM_OBJS = $(patsubst %.asm, build/%.o, $(notdir $(ASM_SOURCES)))

OBJS = $(C_OBJS) $(ASM_OBJS)

all: prepare kernel.bin

prepare:
	mkdir -p build

# Regras para compilar arquivos Assembly (.asm)
build/%.o: kernel/%.asm
	$(ASM) $(ASMFLAGS) $< -o $@

build/%.o: drivers/%.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Regras para compilar arquivos C (.c)
build/auth.o: kernel/auth.c
	$(CC) $(CFLAGS) -c $< -o $@

build/gui_desktop.o: kernel/gui_desktop.c
	$(CC) $(CFLAGS) -c $< -o $@

build/init.o: kernel/init.c
	$(CC) $(CFLAGS) -c $< -o $@

build/kernel.o: kernel/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

build/scheduler.o: kernel/scheduler.c
	$(CC) $(CFLAGS) -c $< -o $@

build/shell.o: kernel/shell.c
	$(CC) $(CFLAGS) -c $< -o $@

build/splash.o: kernel/splash.c
	$(CC) $(CFLAGS) -c $< -o $@

build/tui_desktop.o: kernel/tui_desktop.c
	$(CC) $(CFLAGS) -c $< -o $@

build/window_manager.o: kernel/window_manager.c
	$(CC) $(CFLAGS) -c $< -o $@

build/AC97.o: drivers/AC97.c
	$(CC) $(CFLAGS) -c $< -o $@

build/RTC.o: drivers/RTC.c
	$(CC) $(CFLAGS) -c $< -o $@

build/ehci.o: drivers/ehci.c
	$(CC) $(CFLAGS) -c $< -o $@

build/gdt.o: drivers/gdt.c
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: drivers/idt.c
	$(CC) $(CFLAGS) -c $< -o $@

build/keyboard.o: drivers/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse.o: drivers/mouse.c
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse_gfx.o: drivers/mouse_gfx.c
	$(CC) $(CFLAGS) -c $< -o $@

build/pic.o: drivers/pic.c
	$(CC) $(CFLAGS) -c $< -o $@

build/vbe_glass.o: drivers/vbe_glass.c
	$(CC) $(CFLAGS) -c $< -o $@

build/vga.o: drivers/vga.c
	$(CC) $(CFLAGS) -c $< -o $@

build/leofiles_core.o: fs/leofiles_core.c
	$(CC) $(CFLAGS) -c $< -o $@

build/leofiles_ops.o: fs/leofiles_ops.c
	$(CC) $(CFLAGS) -c $< -o $@

build/leofiles_vfs.o: fs/leofiles_vfs.c
	$(CC) $(CFLAGS) -c $< -o $@

build/memory.o: mm/memory.c
	$(CC) $(CFLAGS) -c $< -o $@

build/paging.o: mm/paging.c
	$(CC) $(CFLAGS) -c $< -o $@

build/elf_loader.o: modules/elf_loader.c
	$(CC) $(CFLAGS) -c $< -o $@

build/hello_mod.o: modules/hello_mod.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.bin build/*.o
	@echo "--------------------------------------------------------"
	@echo "LE-NIDAS OS COMPILADO COM SUCESSO NO TERMUX!"
	@echo "--------------------------------------------------------"

clean:
	rm -rf build/*.o kernel.bin build/kernel.bin

