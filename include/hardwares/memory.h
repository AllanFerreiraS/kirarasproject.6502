#ifndef MEMORY_H_
#define MEMORY_H_

#include "./../utils.h"

#define	MEMORY_TYPE_ROM		0
#define MEMORY_TYPE_RAM		1
#define MEMORY_TARGET_PGR	0
#define MEMORY_TARGET_CHR	1

struct memory {
	u8  *data;
	u8  type;	// ROM or RAM
	u8  target;	// PGR or CHR
	u32 size;
};

enum memory_errno {
	MEMORY_ERRNO_OK = 0,
	MEMORY_ERRNO_WRONG_TYPE,
	MEMORY_ERRNO_WRONG_TARGET,
	MEMORY_ERRNO_ALLOC_FAILED,
	MEMORY_ERRNO_FILE_NULL,
	MEMORY_ERRNO_SEGFAULT,
	MEMORY_ERRNO_WRITE_ROM
};

enum memory_errno memory_alloc(struct memory *m, u8 type, u8 target, u32 size);
void memory_free(struct memory *m);
enum memory_errno memory_rom_burn(struct memory *m, const char *rom, u32 addr_init);
u8   memory_read(struct memory *m, u32 address);
void memory_write(struct memory *m, u32 address, u8 data);
void memory_erase(struct memory *m, u8 data);
#endif

