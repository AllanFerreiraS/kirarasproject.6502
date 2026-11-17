#----------------------------
#	Bin Programs
#----------------------------
CC = gcc
C_DEFINES = -DTESTS_SINGLESTEPTESTS
CFLAGS = -Wall -Wextra -g -fsanitize=address -Iprocessor
#CA65 = ca65
#LD65 = ld65

#----------------------------
#	   Folders
#----------------------------

#----------------------------
#	   SOURCES
#----------------------------
SRC_PATH = ./../../../src
EMULATOR_PATH = ./../../../emulators
THIRDPARTY_PATH = ./../../../thirdparty

SRC_CORE = \
	$(SRC_PATH)/hardwares/cpu6502.c \
	$(SRC_PATH)/hardwares/memory_16.c \
	$(EMULATOR_PATH)/machine_test/machine_test.c

SRC_THIRDPARTY = \
	$(THIRDPARTY_PATH)/cJson/cJSON.c

#----------------------------
#	   Files
#----------------------------
F_C_MAIN = singlesteptests

#----------------------------
#      Make Executables
#----------------------------
all: singlesteptests

singlesteptests:
	$(CC) $(C_DEFINES) $(CFLAGS) $(F_C_MAIN).c $(SRC_CORE) $(SRC_THIRDPARTY) -o ./$(F_C_MAIN)
