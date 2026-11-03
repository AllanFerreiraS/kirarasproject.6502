CC = gcc
CFLAGS = -Wall -Wextra -Iprocessor

CA65 = ca65
LD65 = ld65


SRC = \
	./test.c \
	./../../src/hardwares/processor_6502.c \
	./../../src/hardwares/memory_16.c \
	./../../src/debugger.c \
	./../../src/terminal_ui.c \
	./../../emulators/raw/raw.c

OUT = ./run_test

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	$(OUT)

.PHONY: run
