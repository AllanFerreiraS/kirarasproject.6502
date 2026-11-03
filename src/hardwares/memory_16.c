#include "./../../include/hardwares/memory_16.h"
#include <stdio.h>

#define TRUE	1
#define	FALSE	0

uint8_t memory_read(memory_16 * memory, uint16_t address)
{
	return memory->data[address];
}

void memory_write(memory_16 * memory, uint16_t address, uint8_t data)
{

	memory->data[address] = data;
}

uint8_t write_rom(memory_16 * memory, const char *rom_bin, uint16_t addr_start)
{

	FILE *file = fopen(rom_bin, "rb");
	if (file == NULL)
		return FALSE;
	fread(&(memory->data[addr_start]), 1, MEMORY_16_SIZE - addr_start, file);
	fclose(file);
	return TRUE;
}

void print_zero_page(memory_16 * memory)
{
	for (uint16_t i = 0x00; i < 0x100; i++) {
		if (i % 16 == 0)
			printf("\n%x\t", i);
		printf("%02x  ", memory->data[i]);
	}
	printf("\n");
}
