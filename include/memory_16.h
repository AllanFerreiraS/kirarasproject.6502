#ifndef MEMORY_16_H_
#define MEMORY_16_H_

#include <stdint.h>

#define MEMORY_16_SIZE 0x10000 //2^16

typedef struct {
	uint8_t data[MEMORY_16_SIZE];
} memory_16;

// Public Functions
uint8_t memory_read(memory_16 * memory, uint16_t address);
void	memory_write(memory_16 * memory, uint16_t address, uint8_t data);
void	write_rom(memory_16 * memory, const char *rom_bin);
void 	print_zero_page(memory_16 * memory);
#endif
