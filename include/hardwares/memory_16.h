#ifndef MEMORY_16_H_
#define MEMORY_16_H_

#include "./../utils.h"

#define	MEMORY_ROM	0
#define MEMORY_RAM	1

struct memory {
	u8  *data;
	u8  type;
	u32 size;
};

u8   memory_alloc(struct memory *m, u8 type, u32 size);
void memory_free(struct memory *m);
u8   memory_read(struct memory *m, u32 address);
void memory_write(struct memory *m, u32 address, u8 data);
u8   memory_rom_burn(struct memory *m, const char *rom, u32 addr_start);
void memory_clean(struct memory *m);
#endif
