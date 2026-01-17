# Компиляторы
ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
DD = dd

# Флаги
ASM_FLAGS = -f bin
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-builtin -O0 -Iinclude
LDFLAGS = -m elf_i386 -T linker.ld -nostdlib

# Файлы
BOOT_SRC = boot/boot.asm
BIOSMENU_SRC = src/biosmenu.c
POST_SRC = src/post.c
CONSOLE_SRC = src/console.c
EFFICIENCY_SRC = src/efficiency.c
CPU_SRC = src/cpu.c
rtc_SRC = src/rtc.c  # Добавили rtc

# Выходные файлы
BIN_DIR = bin
BOOT_BIN = $(BIN_DIR)/boot.bin
BIOSMENU_BIN = $(BIN_DIR)/biosmenu.bin
BIOSMENU_ELF = $(BIN_DIR)/biosmenu.elf

# Объектные файлы
BIOSMENU_O = $(BIN_DIR)/biosmenu.o
POST_O = $(BIN_DIR)/post.o
CONSOLE_O = $(BIN_DIR)/console.o
EFFICIENCY_O = $(BIN_DIR)/efficiency.o
CPU_O = $(BIN_DIR)/cpu.o
rtc_O = $(BIN_DIR)/rtc.o  # Объектный файл rtc

IMG = $(BIN_DIR)/bios.img

# Цели
all: $(IMG)

# Создание образа
$(IMG): $(BOOT_BIN) $(BIOSMENU_BIN)
	@mkdir -p $(BIN_DIR)
	$(DD) if=/dev/zero of=$(IMG) bs=512 count=2880 status=none
	$(DD) if=$(BOOT_BIN) of=$(IMG) conv=notrunc status=none
	$(DD) if=$(BIOSMENU_BIN) of=$(IMG) bs=512 seek=1 conv=notrunc status=none

# Загрузчик
$(BOOT_BIN): $(BOOT_SRC)
	@mkdir -p $(BIN_DIR)
	$(ASM) $(ASM_FLAGS) $(BOOT_SRC) -o $(BOOT_BIN)

# C код BIOS
$(BIOSMENU_BIN): $(BIOSMENU_ELF)
	$(OBJCOPY) -O binary $(BIOSMENU_ELF) $(BIOSMENU_BIN)

# ЛИНКОВКА - добавлен rtc_O
$(BIOSMENU_ELF): $(BIOSMENU_O) $(POST_O) $(CONSOLE_O) $(EFFICIENCY_O) $(CPU_O) $(rtc_O) linker.ld
	$(LD) $(LDFLAGS) -o $(BIOSMENU_ELF) $(BIOSMENU_O) $(POST_O) $(CONSOLE_O) $(EFFICIENCY_O) $(CPU_O) $(rtc_O)

# Правила компиляции

$(BIOSMENU_O): $(BIOSMENU_SRC) include/stdint.h include/post.h include/console.h include/efficiency.h include/cpu.h include/rtc.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(BIOSMENU_SRC) -o $(BIOSMENU_O)

$(POST_O): $(POST_SRC) include/post.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(POST_SRC) -o $(POST_O)

$(CONSOLE_O): $(CONSOLE_SRC) include/console.h include/post.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(CONSOLE_SRC) -o $(CONSOLE_O)

$(EFFICIENCY_O): $(EFFICIENCY_SRC) include/efficiency.h include/console.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(EFFICIENCY_SRC) -o $(EFFICIENCY_O)

# Правило для cpu.c
$(CPU_O): $(CPU_SRC) include/cpu.h include/console.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(CPU_SRC) -o $(CPU_O)

# ДОБАВЛЕНО: Правило для rtc.c
$(rtc_O): $(rtc_SRC) include/rtc.h include/console.h include/cpu.h
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $(rtc_SRC) -o $(rtc_O)

# Очистка
clean:
	rm -rf $(BIN_DIR)
	
run:
	qemu-system-x86_64 -drive format=raw,file=bin/bios.img -net none

.PHONY: all clean run
