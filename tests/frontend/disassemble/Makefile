#----------------------------
#	Bin Programs
#----------------------------
CC = gcc
# C_DEFINES
CFLAGS = -Wall -Wextra -g -fsanitize=address -Iprocessor
CA65 = ca65
LD65 = ld65
RM = rm

#----------------------------
#	   Folders
#----------------------------
BUILD_DIR = ./build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

#----------------------------
#	   SOURCES
#----------------------------
SRC_PATH = ./../../../src
FRONTEND_PATH = ./../../../frontend

SRC_CORE = \
	$(SRC_PATH)/hardwares/cpu6502.c \
	$(SRC_PATH)/hardwares/memory.c

SRC_FRONTEND = \
	$(FRONTEND_PATH)/disassemble/disassemble.c

#----------------------------
#	   Files
#----------------------------
F_C_MAIN = test_disasm
F_ASM = disasm

#----------------------------
#      Make Executables
#----------------------------
all: test_disasm rom_disasm

test_disasm: $(BUILD_DIR)
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) $(SRC_FRONTEND) -o $(BUILD_DIR)/$(F_C_MAIN)

rom_disasm: $(BUILD_DIR)
	$(CA65) -l $(F_ASM).lst $(F_ASM).ca65
	$(LD65) $(F_ASM).o -o $(BUILD_DIR)/$(F_ASM).bin -C $(F_ASM).cfg
	rm ./*.o
clean:
	rm -r $(BUILD_DIR)/*
