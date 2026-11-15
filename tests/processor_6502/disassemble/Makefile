#----------------------------
#	Bin Programs 
#----------------------------
CC = gcc
# C_DEFINES
CFLAGS = -Wall -Wextra -Iprocessor
CA65 = ca65
LD65 = ld65
RM = rm

#----------------------------
#	   Folders 
#----------------------------
BUILD_DIR = build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

#----------------------------
#	   SOURCES
#----------------------------
# Hardware
SRC_CORE = \
	./../../../src/hardwares/processor_6502.c \
	./../../../src/hardwares/memory_16.c \
	./../../../src/disassemble.c
# Debugger
# SRC_DEBUGGER = \

#----------------------------
#	   Files
#----------------------------
F_C_MAIN = disasm_test
F_ASM = disasm

#----------------------------
#      Make Executables
#----------------------------
all:
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(BUILD_DIR)/$(F_C_MAIN)
	$(CA65) -l $(F_ASM).lst $(F_ASM).ca65
	$(LD65) $(F_ASM).o -o $(BUILD_DIR)/$(F_ASM).bin -C $(F_ASM).cfg
	rm ./*.o
clean:
	rm -rf $(BUILD_DIR)/*
