CC = gcc
CFLAGS = -Wall -Wextra -g -fsanitize=address -Iprocessor

SRC = \
	./singlesteptests.c \
	../../src/processor_6502.c \
	../../src/memory_16.c \
	../../thirdparty/cJson/cJSON.c

OUT = ./run_singlesteptests

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	$(OUT)

.PHONY: run
