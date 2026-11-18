#include "./nes.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

u8 nes_prepare_rom(struct nes *emulator, const char *bin, u16 memory_start)
{
	if (! write_rom(&(emulator->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		return FALSE;
	}
	return TRUE;
}

void nes_prepare_cpu(struct machine_test *emulator)
{
	emulator->cpu.bus = (buses){ machine_test_read, machine_test_write, emulator };
}

/*
#ifdef FRONTEND_DEBUGGER
void nes_monitor_run(struct nes *emulator, u8 step_by_step, const char * bin, u16 memory_start)
{
	if (! machine_test_prepare_rom(emulator, bin, memory_start))
		exit(EXIT_FAILURE);

	disassemble_asm_list asm_list;
	asm_list = disassemble(emulator->memory.data, MEMORY_16_SIZE);

	machine_test_prepare_cpu(emulator);
	cpu6502_init(&(emulator->cpu), CPU6502_POWER_UP);

	debugger_terminal_prepare();
	debugger_terminal_run(&(emulator->cpu), &(emulator->memory), &asm_list, step_by_step);
	while(1) {

		cpu6502_opcode_execute(&(emulator->cpu));
		debugger_terminal_run(&(emulator->cpu), &(emulator->memory), &asm_list, step_by_step);
		cpu6502_interrupt_handle_request_if_any(&(emulator->cpu));
	}
	debugger_terminal_finish();
}
#endif
*/

void nes_run(struct nes *emulator, const char * bin, u16 memory_start)
{
	if (! machine_test_prepare_rom(emulator, bin, memory_start))
		exit(EXIT_FAILURE);

	machine_test_prepare_cpu(emulator);
	cpu6502_init(&(emulator->cpu), CPU6502_POWER_UP);
	while(1) {
		cpu6502_opcode_execute(&(emulator->cpu));
		cpu6502_interrupt_handle_request_if_any(&(emulator->cpu));
	}
}

u8 nes_read(void *emulator_void, u16 address)
{
	struct machine_test *emulator = emulator_void;
	u8 data = 0;

	emulator->cpu.cycles++;
	emulator->cpu.read_write = 0;

	data = memory_read(&(emulator->memory), address);
	return data;
}

void nes_write(void *emulator_void, u16 address, u8 data)
{
	struct machine_test *emulator = emulator_void;

	emulator->cpu.cycles++;
	emulator->cpu.read_write = 1;

	memory_write(&(emulator->memory), address, data);
}
