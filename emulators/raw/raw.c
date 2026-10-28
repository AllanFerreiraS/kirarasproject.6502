#include <stdint.h>
#include <stdio.h>
#include "./raw.h"
#include "./../../include/infrastructure.h"
#include "./../../include/buses.h"
#include "./../../include/hardwares/processor_6502.h"
#include "./../../include/hardwares/memory_16.h"

#define DEBUG

uint16_t c_cycles[3 * 1256] = {0};
uint16_t c_cycles_i = 0;

uint8_t bus_raw_read(void *infra, uint16_t address);
void	bus_raw_write(void *infra, uint16_t address, uint8_t data);

void raw_run(infra_raw * e)
{
	char stop_flag;

	write_rom(&(e->memory), "./tests/rom.bin");
	init_6502(&(e->cpu));

	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };

	while(1) {
		execute_opcode(&(e->cpu), &(e->memory));
		scanf("%c", &stop_flag);
		if (stop_flag == 'q')
			break;
	}
}

uint8_t bus_raw_read(void *infra, uint16_t address)
{
	infra_raw *emu = infra;
	uint8_t data = 0;

	emu->cpu.cycles++;
	emu->cpu.read_write = 0;

	data = memory_read(&(emu->memory), address);

	#ifdef DEBUG
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = 0;
	#endif
	return data;
}

void bus_raw_write(void *infra, uint16_t address, uint8_t data)
{
	infra_raw *emu = infra;
	
	emu->cpu.cycles++;
	emu->cpu.read_write = 1;

	#ifdef DEBUG
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = 1;
	#endif
	memory_write(&(emu->memory), address, data);
}

