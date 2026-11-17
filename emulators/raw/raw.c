#include "./raw.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

uint16_t c_cycles[3 * 256] = {0};
uint16_t c_cycles_i = 0;

#ifdef FRONTEND_DEBUGGER
void raw_run_with_monitor(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t memory_start)
{
	disassemble_asm_list asm_list;

	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}

	asm_list = disassemble(e->memory.data, MEMORY_16_SIZE);

	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };
	cpu6502_init(&(e->cpu), CPU6502_POWER_UP);

	debugger_terminal_prepare();
	debugger_terminal_run(&(e->cpu), &(e->memory), &asm_list, step_by_step);
	while(1) {

		cpu6502_opcode_execute(&(e->cpu));
		debugger_terminal_run(&(e->cpu), &(e->memory), &asm_list, step_by_step);
		cpu6502_interrupt_handle_request_if_any(&(e->cpu));
	}
	debugger_terminal_finish();
}
#endif


void raw_run(infra_raw * e, const char * bin, uint16_t memory_start)
{
	if (! write_rom(&(e->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		exit(EXIT_FAILURE);
	}

	e->cpu.bus = (buses){ bus_raw_read, bus_raw_write, e };
	cpu6502_init(&(e->cpu), CPU6502_POWER_UP);

	while(1) {
		cpu6502_opcode_execute(&(e->cpu));
		cpu6502_interrupt_handle_request_if_any(&(e->cpu));
	}
}

uint8_t bus_raw_read(void *infra, uint16_t address)
{
	infra_raw *emu = infra;
	uint8_t data = 0;

	emu->cpu.cycles++;
	emu->cpu.read_write = 0;

	data = memory_read(&(emu->memory), address);

	#ifdef TESTS_SINGLESTEPTESTS
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

	#ifdef TESTS_SINGLESTEPTESTS
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = 1;
	#endif
	memory_write(&(emu->memory), address, data);


	if (address == 0xBFFC) {
		emu->cpu.irq_pin = ((data & 1) > 0) ? CPU6502_IRQ_LOW : CPU6502_IRQ_HIGH;
		emu->cpu.nmi_pin = (((data >> 1) & 1) > 0) ? CPU6502_NMI_LOW : CPU6502_NMI_HIGH;
	}
}

