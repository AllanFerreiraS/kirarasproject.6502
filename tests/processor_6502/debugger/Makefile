#----------------------------
#	Bin Programs 
#----------------------------
CC = gcc
# C_DEFINES
CFLAGS = -Wall -Wextra -Iprocessor
CA65 = ca65
LD65 = ld65

#----------------------------
#	   Folders 
#----------------------------
D_BUILD = ./build
D_LINKERS = ./linkers
D_ROMS = ./roms
MKDIRS = $(D_BUILD) $(D_LINKERS) $(D_ROMS)

$(MKDIRS):
	mkdir -p $@

#----------------------------
#	   SOURCES
#----------------------------
SRC_PATH = ../../../src
EMULATOR_PATH = ../../../emulators

SRC_CORE = \
	$(SRC_PATH)/hardwares/processor_6502.c \
	$(SRC_PATH)/hardwares/memory_16.c \
	$(SRC_PATH)/terminal_ui.c \
	$(SRC_PATH)/debugger.c \
	$(SRC_PATH)/disassemble.c \
	$(EMULATOR_PATH)/raw/raw.c
# Debugger
# SRC_DEBUGGER = \

#----------------------------
#	   Files
#----------------------------
F_C_MAIN = test_debugger
F_ROM_1 = rom_1

#----------------------------
#      Make Executables
#----------------------------
all: test_rom_1

test_debugger: $(MKDIRS)
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(D_BUILD)/$(F_C_MAIN)

rom_1: $(MKDIRS)
	$(CA65) -l $(D_ROMS)/$(F_ROM_1).lst $(D_ROMS)/$(F_ROM_1).ca65
	$(LD65) $(D_ROMS)/$(F_ROM_1).o -o $(D_BUILD)/$(F_ROM_1).bin -C $(D_LINKERS)/$(F_ROM_1).cfg
	rm $(D_ROMS)/*.o

clean:
	rm -rf $(D_BUILD)/*
