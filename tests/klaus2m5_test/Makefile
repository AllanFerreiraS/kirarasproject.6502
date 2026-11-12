CC = gcc
CFLAGS = -Wall -Wextra -Iprocessor

CA65 = ca65
LD65 = ld65


SRC = \
	./disassemble_test.c \
	./../../src/hardwares/processor_6502.c \
	./../../src/hardwares/memory_16.c \
	./../../src/disassemble.c

OUT = ./run_test

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	$(OUT)

.PHONY: run
