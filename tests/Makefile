#----------------------------
#	Bin Programs
#----------------------------
CC = gcc
#C_DEFINES
CFLAGS = -Wall -Wextra -g -fsanitize=address -Iprocessor

#----------------------------
#	   Folders
#----------------------------

#----------------------------
#	   SOURCES
#----------------------------
SRC_PATH = ./../src
EMULATOR_PATH = ./../emulators
FRONTEND_PATH = ./../frontend

SRC_CORE = \
	$(SRC_PATH)/buses.c \
	$(SRC_PATH)/hardwares/cpu6502.c \
	$(SRC_PATH)/hardwares/memory.c
#	$(EMULATOR_PATH)/machine_test/machine_test.c

#SRC_FRONTEND = \
	$(FRONTEND_PATH)/terminal_ui/terminal_ui.c \
	$(FRONTEND_PATH)/debugger/debugger.c \
	$(FRONTEND_PATH)/disassemble/disassemble.c


#----------------------------
#	   Files
#----------------------------
F_C_MAIN = test

#----------------------------
#      Make Executables
#----------------------------
all: build

build: $(MKDIRS)
	$(CC) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) -o $(F_C_MAIN)
	ca65 rom.s
	ld65 -t none rom.o -o rom
