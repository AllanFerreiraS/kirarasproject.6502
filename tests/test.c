#include "./../include/hardwares/cpu6502.h"
#include "./../include/hardwares/memory.h"
#include <stdio.h>

// Global just for facility
struct	cpu6502 cpu;
struct	memory rom;
struct  memory ram;

// Check the rom size before storing it to the memory



u8 readbus(void *emulator, u16 addr);
void writebus(void *emulator, u16 addr, u8 data);

int main() {

	//struct	cpu6502 cpu;
	//struct	memory rom;
	//struct	memory ram;
	enum	memory_errno memsts;

	const char *romfile = "./rom";

	memsts = memory_alloc(&rom, MEMORY_TYPE_ROM, MEMORY_TARGET_PGR, U16_SIZE);

	if (memsts == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EXIT_FAILURE);
	} else if (memsts == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "Wrong Memory Type (ROM or RAM).\n");
		exit(EXIT_FAILURE);
	} else if (memsts == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "Wrong Memory Target (PRG or CHR).\n");
		exit(EXIT_FAILURE);
	}

	memsts = memory_rom_burn(&rom, romfile, 0x00);

	if (memsts == MEMORY_ERRNO_SEGFAULT) {
		fprintf(stderr, "SEGFAULT.\n");
		exit(EXIT_FAILURE);
	} else if (memsts == MEMORY_ERRNO_FILE_NULL) {
		fprintf(stderr, "Error opening rom file\n");
	}

	memsts = memory_alloc(&ram, MEMORY_TYPE_RAM, MEMORY_TARGET_PGR, U16_SIZE);

	if (memsts == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "Memory allocation failed.\n");
		exit(EXIT_FAILURE);
	} else if (memsts == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "Wrong Memory Type (ROM or RAM).\n");
		exit(EXIT_FAILURE);
	} else if (memsts == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "Wrong Memory Target (PRG or CHR).\n");
		exit(EXIT_FAILURE);
	}

	if (cpu6502_bus_prepare(&cpu, readbus, writebus, &cpu) == CPU6502_ERRNO_BUS_FUNCTION_NULL) {
		fprintf(stderr, "Bus Read and/or Bus Write pointers are NULL\n");
		exit(EXIT_FAILURE);
	}

	cpu6502_init(&cpu, CPU6502_POWER_UP);

	for (u32 i = 0; i < 0x20; i++)
		printf("0x%08x: 0x%08x\n", i, memory_read(&rom, i));

	for (u8 i = 0; i < 0x12; i++) {
		printf("PC:%02x\n", cpu6502_regpc(&cpu));
		printf("A: %02x\n", cpu6502_rega(&cpu));

		if (cpu6502_opcode_execute(&cpu) == CPU6502_ERRNO_ILLEGAL_OPCODE) {
			fprintf(stderr,	"Illegal Opcode!\n");
			exit(EXIT_FAILURE);
		}
	}


	for (u32 i = 0x20; i < 0x2f; i++)
		printf("0x%08x: 0x%08x\n", i, memory_read(&ram, i));

	memory_free(&rom);
	memory_free(&ram);
	return 0;
}





u8 readbus(void *emulator, u16 addr)
{
	struct cpu6502 *cpu = emulator;
	cpu->cycles++;
	cpu->read_write = CPU6502_FLAG_READ;

	if ( buses_addr_in_range(addr, 0x0000, 0x001f) ) {
		return memory_read(&rom, addr);
	} else if ( buses_addr_in_range(addr, 0x0020, 0x002f) ) {
		return memory_read(&ram, addr);
	} else {
		return buses_openbus(&(cpu->bus));
	}
}

void writebus(void *emulator, u16 addr, u8 data)
{
	struct cpu6502 *cpu = emulator;
	cpu->cycles++;
	cpu->read_write = CPU6502_FLAG_WRITE;

	if ( buses_addr_in_range(addr, 0x0000, 0x001f) ) {
		memory_write(&rom, addr, data);
		buses_update_previous_data(&(cpu->bus), data);
	} else if ( buses_addr_in_range(addr, 0x0020, 0x002f) ){
		addr = buses_mirror(addr, 0x0020, 4);
		memory_write(&ram, addr, data);
		buses_update_previous_data(&(cpu->bus), data);
	}
}
