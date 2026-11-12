CC = gcc
CFLAGS = -Wall -Wextra -Iprocessor

CA65 = ca65
LD65 = ld65

ASM_SOURCE = roms/rom.s
ASM_OBJECT = rom_objs/rom.o
ROM = bins/rom.bin
LINKER_CONFIG = linkers/linker.cfg

SRC = \
	./rom.c \
	../../src/hardwares/processor_6502.c \
	../../src/hardwares/memory_16.c \
	../../src/terminal_ui.c \
	../../src/debugger.c \
	../../emulators/raw/raw.c

OUT = run_rom

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
	$(CA65) $(ASM_SOURCE) -o $(ASM_OBJECT)
	$(LD65) $(ASM_OBJECT) -C $(LINKER_CONFIG) -o $(ROM)

run: all
	$(OUT)

.PHONY: run
