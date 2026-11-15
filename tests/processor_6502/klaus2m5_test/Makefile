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
	./../../../emulators/raw/raw.c \
	./../../../src/terminal_ui.c \
	./../../../src/debugger.c \
	./../../../src/disassemble.c
# Debugger
# SRC_DEBUGGER = \

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
all: test_interrupt test_functional test_decimal

test_interrupt:
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(BUILD_DIR)/$(F_C_MAIN)
	$(CA65) -l $(F_ASM_INTERRUPT).lst $(F_ASM_INTERRUPT).ca65
	$(LD65) $(F_ASM_INTERRUPT).o -o $(BUILD_DIR)/$(F_ASM_INTERRUPT).bin -C $(F_KLAUS_CFG)
	rm ./*.o

test_functional:
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(BUILD_DIR)/$(F_C_MAIN)
	$(CA65) -l $(F_ASM_FUNCTIONAL).lst $(F_ASM_FUNCTIONAL).ca65
	$(LD65) $(F_ASM_FUNCTIONAL).o -o $(BUILD_DIR)/$(F_ASM_FUNCTIONAL).bin -C $(F_KLAUS_CFG)
	rm ./*.o

test_decimal:
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(BUILD_DIR)/$(F_C_MAIN)
	$(CA65) -l $(F_ASM_DECIMAL).lst $(F_ASM_DECIMAL).ca65
	$(LD65) $(F_ASM_DECIMAL).o -o $(BUILD_DIR)/$(F_ASM_DECIMAL).bin -C $(F_KLAUS_CFG)
	rm ./*.o

clean:
	rm -rf $(BUILD_DIR)/*
