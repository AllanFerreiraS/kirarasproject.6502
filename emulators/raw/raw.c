#include "./raw.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//define DEBUG_SINGLESTEPTESTS

uint16_t c_cycles[3 * 256] = {0};
uint16_t c_cycles_i = 0;


void raw_run(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t pc_start, uint16_t memory_start)
{
	char stop_flag;

	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}
	
	init_6502(&(e->cpu), pc_start);
	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };

	while(1) {
		execute_opcode(&(e->cpu), &(e->memory));

		if (step_by_step) {
			scanf("%c", &stop_flag);
			if (stop_flag == 'q')
				break;
		}
	}
}


void raw_run_with_monitor(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t pc_start, uint16_t memory_start)
{
	char stop_flag;

	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}

	init_6502(&(e->cpu), pc_start);
	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };

	debugger_terminal_prepare();
	while(1) {

		debugger_run(&(e->memory), &(e->cpu));
		fflush(stdout);
		execute_opcode(&(e->cpu), &(e->memory));
		
		if (step_by_step) {
			scanf("%c", &stop_flag);
			if (stop_flag == 'q')
				break;
		} else {
			usleep(11 * 1000);
		}
	}
	debugger_terminal_finish();
}

uint8_t bus_raw_read(void *infra, uint16_t address)
{
	infra_raw *emu = infra;
	uint8_t data = 0;

	emu->cpu.cycles++;
	emu->cpu.read_write = 0;

	data = memory_read(&(emu->memory), address);

	#ifdef DEBUG_SINGLESTEPTESTS
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

	#ifdef DEBUG_SINGLESTEPTESTS
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = 1;
	#endif
	memory_write(&(emu->memory), address, data);
}

