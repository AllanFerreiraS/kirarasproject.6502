#----------------------------
#	Bin Programs
#----------------------------
CC = gcc
# C_DEFINES = -DFRONTEND_DEBUGGER
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
EMULATOR_PATH = ./../../../emulators

SRC_CORE = \
	$(SRC_PATH)/hardwares/cpu6502.c \
	$(SRC_PATH)/hardwares/memory.c \
	$(EMULATOR_PATH)/machine_test/machine_test.c

#----------------------------
#	   Files
#----------------------------
F_C_MAIN = test_klaus2m5
F_ASM_INTERRUPT = 6502_interrupt_test
F_ASM_FUNCTIONAL = 6502_functional_test
F_ASM_DECIMAL = 6502_decimal_test
F_KLAUS_CFG = klaus2m5_test.cfg

#----------------------------
#      Make Executables
#----------------------------
all: test_klaus2m5 rom_interrupt rom_functional rom_decimal

test_klaus2m5:
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(BUILD_DIR)/$(F_C_MAIN)

rom_interrupt:
	$(CA65) -l $(F_ASM_INTERRUPT).lst $(F_ASM_INTERRUPT).ca65
	$(LD65) $(F_ASM_INTERRUPT).o -o $(BUILD_DIR)/$(F_ASM_INTERRUPT).bin -C $(F_KLAUS_CFG)
	rm ./*.o

rom_functional:
	$(CA65) -l $(F_ASM_FUNCTIONAL).lst $(F_ASM_FUNCTIONAL).ca65
	$(LD65) $(F_ASM_FUNCTIONAL).o -o $(BUILD_DIR)/$(F_ASM_FUNCTIONAL).bin -C $(F_KLAUS_CFG)
	rm ./*.o

rom_decimal:
	$(CA65) -l $(F_ASM_DECIMAL).lst $(F_ASM_DECIMAL).ca65
	$(LD65) $(F_ASM_DECIMAL).o -o $(BUILD_DIR)/$(F_ASM_DECIMAL).bin -C $(F_KLAUS_CFG)
	rm ./*.o

clean:
	rm -rf $(BUILD_DIR)/*
