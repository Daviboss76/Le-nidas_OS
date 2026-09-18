CC = clang
ASM = nasm
LD = ld.lld

CFLAGS = --target=i686-pc-none-elf -march=i686 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -fno-builtin -fno-stack-protector -nostdlib -ffreestanding -mno-red-zone -Iinclude
ASMFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T linker.ld

# Diz ao make onde procurar os arquivos fonte nas subpastas
vpath %.c kernel drivers fs mm modules
vpath %.asm kernel drivers

# Acha automaticamente todos os arquivos .c e .asm nas pastas do projeto
C_SOURCES = $(wildcard kernel/*.c drivers/*.c fs/*.c mm/*.c modules/*.c)
ASM_SOURCES = $(wildcard kernel/*.asm drivers/*.asm)

# Transforma os nomes dos arquivos em caminhos dentro da pasta build/
C_OBJS = $(patsubst %.c, build/%.o, $(notdir $(C_SOURCES)))
ASM_OBJS = $(patsubst %.asm, build/%.o, $(notdir $(ASM_SOURCES)))

OBJS = $(C_OBJS) $(ASM_OBJS)

all: prepare kernel.bin

prepare:
	mkdir -p build

# Regra genérica para compilar qualquer arquivo .c de qualquer pasta
build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra genérica para compilar qualquer arquivo .asm de qualquer pasta
build/%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

kernel.bin: $(OBJS) build/audio_hino.o build/autoreparar.o
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJS) build/audio_hino.o build/autoreparar.o
	@echo "--------------------------------------------------------"
	@echo "LE-NIDAS OS COMPILADO COM SUCESSO NO TERMUX!"
	@echo "--------------------------------------------------------"

# Regra para converter o binário do áudio em objeto ELF 32-bit automaticamente
audio_hino.o: audio_hino.bin
	ld.lld -m elf_i386 -r -b binary audio_hino.bin -o audio_hino.o
clean:
	rm -rf build/*.o kernel.bin


build/audio_hino.o build/autoreparar.o: audio_hino.bin
	clang --target=i686-pc-none-elf -march=i686 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -fno-builtin -fno-stack-protector -nostdlib -ffreestanding -mno-red-zone -Iinclude -c autoreparar/main.c -o build/autoreparar.o
	ld.lld -m elf_i386 -r -b binary audio_hino.bin -o build/audio_hino.o

build/autoreparar.o: autoreparar/main.c
	clang --target=i686-pc-none-elf -march=i686 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -fno-builtin -fno-stack-protector -nostdlib -ffreestanding -mno-red-zone -Iinclude -c autoreparar/main.c -o build/autoreparar.o
