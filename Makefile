CC = gcc
CFLAGS = -Wall -Wextra -Iprocessor

CA65 = ca65
LD65 = ld65

ASM_SOURCE = tests/rom.s
ASM_OBJECT = tests/rom.o
ROM = tests/rom.bin
LINKER_CONFIG = tests/linker.cfg

SRC = \
	./main.c \
	./src/processor_6502.c \
	./src/memory_16.c \
	./src/tui.c

OUT = ./bin/6502

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
	$(CA65) $(ASM_SOURCE) -o $(ASM_OBJECT)
	$(LD65) $(ASM_OBJECT) -C $(LINKER_CONFIG) -o $(ROM)

run: all
	$(OUT)

.PHONY: run
