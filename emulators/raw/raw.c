#include "./raw.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Debug
//#define DEBUG_SINGLESTEPTESTS



uint16_t c_cycles[3 * 256] = {0};
uint16_t c_cycles_i = 0;


void raw_run(infra_raw * e, const char * bin, uint16_t memory_start)
{
	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}

	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };
	processor_6502_initialization(&(e->cpu), PROCESSOR_6502_POWER_UP);

	while(1) {
		processor_6502_execute_opcode(&(e->cpu));
	}
}

void raw_run_with_monitor(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t memory_start)
{
	char stop_flag;

	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}

	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };
	processor_6502_initialization(&(e->cpu), PROCESSOR_6502_POWER_UP);

	debugger_terminal_prepare();
	while(1) {
	
		processor_6502_execute_opcode(&(e->cpu));
		debugger_run(&(e->memory), &(e->cpu));
		fflush(stdout);

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

	uint16_t i_port = 0xbffc;

	#ifdef DEBUG_SINGLESTEPTESTS
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = 1;
	#endif
	memory_write(&(emu->memory), address, data);

	if (address == i_port) {
		emu->cpu.irq_pin = ((data & 0x01) > 0) ? 0 : 1;
		emu->cpu.nmi_request = data & 0x02;
	}

}

