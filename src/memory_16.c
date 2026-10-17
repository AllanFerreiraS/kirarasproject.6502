#include "../include/memory_16.h"
#include <stdio.h>

uint8_t memory_read(memory_16 * memory, uint16_t address)
{
	return memory->data[address];
}

void memory_write(memory_16 * memory, uint16_t address, uint8_t data)
{

	memory->data[address] = data;
}

void write_rom(memory_16 * memory, const char *rom_bin)
{

	FILE *file = fopen(rom_bin, "rb");
	fread(&(memory->data[0x8000]), 1, MEMORY_16_SIZE - 0x8000, file);
	fclose(file);
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
