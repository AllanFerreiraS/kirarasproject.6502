CC = gcc
CFLAGS = -Wall -Wextra -Iprocessor

CA65 = ca65
LD65 = ld65

ASM_SOURCE = tests/rom.s
ASM_OBJECT = tests/rom.o
ROM = tests/rom.bin
LINKER_CONFIG = tests/linker.cfg

SRC = \
	./kirarasproject.6502.c \
	./src/hardwares/processor_6502.c \
	./src/hardwares/memory_16.c \
	./emulators/raw/raw.c

OUT = ./bin/6502

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
	$(CA65) $(ASM_SOURCE) -o $(ASM_OBJECT)
	$(LD65) $(ASM_OBJECT) -C $(LINKER_CONFIG) -o $(ROM)

run: all
	$(OUT)

.PHONY: run
