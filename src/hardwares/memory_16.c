#include "./../../include/hardwares/memory_16.h"
#include <stdio.h>
#include <stdlib.h>

u8 memory_alloc(struct memory *m, u8 type, u32 size)
{
	u8 *data = malloc(size);
	if (data == NULL)
		return FALSE;

	m->data = data;
	m->type = type;
	m->size = size;
	return TRUE;
}

void memory_free(struct memory *m)
{
	free(m->data);
}

u8 memory_read(struct memory *m, u32 address)
{
	return m->data[address];
}

void memory_write(struct memory *m, u32 address, u8 data)
{
	if (m->type == MEMORY_RAM)
		m->data[address] = data;
}

u8 memory_rom_burn(struct memory *m, const char *rom, u32 addr_start)
{
	FILE *file = fopen(rom, "rb");
	if (file == NULL)
		return FALSE;
	fread(&(m->data[addr_start]), 1, m->size - addr_start, file);
	fclose(file);
	return TRUE;
}

void memory_clean(struct memory *m)
{
	for (u32 i = 0; i < m->size; i++)
		m->data[i] = 0;
}
