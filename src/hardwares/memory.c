#include "./../../include/hardwares/memory.h"
#include <stdio.h>

#define MEMORY_LABEL_FATAL_ERROR	"[Memory Fatal Error]:"

enum memory_errno memory_alloc(struct memory *m, u8 type, u8 target, u32 size)
{
	if (type > 1)
		return MEMORY_ERRNO_WRONG_TYPE;

	if (target > 1)
		return MEMORY_ERRNO_WRONG_TARGET;

	u8 *data = malloc(size);

	if (data == NULL)
		return MEMORY_ERRNO_ALLOC_FAILED;

	m->data = data;
	m->type = type;
	m->size = size;

	memory_erase(m, 0x00);

	return  MEMORY_ERRNO_OK;
}

void memory_free(struct memory *m)
{
	free(m->data);
}

enum memory_errno memory_rom_burn(struct memory *m, const char *rom, u32 addr_init)
{
	if (addr_init > (m->size - 1))
		return MEMORY_ERRNO_SEGFAULT;

	FILE *file = fopen(rom, "rb");

	if (file == NULL)
		return MEMORY_ERRNO_FILE_NULL;

	fread(&(m->data[addr_init]), 1, m->size - addr_init, file);
	fclose(file);
	return MEMORY_ERRNO_OK;
}

u8 memory_read(struct memory *m, u32 address)
{
	if ((m->size - 1) < address) {
		fprintf(stderr, "%s Memory Address 0x%08x is bigger compared to the memory last address 0x%08x.\n", MEMORY_LABEL_FATAL_ERROR, address, (m->size - 1));
		exit(EXIT_FAILURE);
	}
	return m->data[address];
}

void memory_write(struct memory *m, u32 address, u8 data)
{
	if ((m->size - 1) < address) {
		fprintf(stderr, "%s Memory Address 0x%08x is bigger compared to the memory last address 0x%08x.\n", MEMORY_LABEL_FATAL_ERROR, address, (m->size - 1));
		exit(EXIT_FAILURE);
	}

	if (m->type == MEMORY_TYPE_RAM)
		m->data[address] = data;
}

void memory_erase(struct memory *m, u8 data)
{
	for (u32 i = 0; i < m->size; i++)
		m->data[i] = data;
}
