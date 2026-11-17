#include "./machine_test.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Used for SingleStepTest Only. TESTS_SINGLESTEPTESTS should be defined.
uint16_t c_cycles[3 * 256] = {0};
uint16_t c_cycles_i = 0;

u8 machine_test_prepare_rom(struct machine_test *emulator, const char *bin, u16 memory_start)
{
	if (! write_rom(&(emulator->memory), bin, memory_start) ) {
		printf("Error: \'%s\' not found.\n", bin);
		return FALSE;
	}
	return TRUE;
}

void machine_test_prepare_cpu(struct machine_test *emulator)
{
	emulator->cpu.bus = (buses){ machine_test_read, machine_test_write, emulator };
}

#ifdef FRONTEND_DEBUGGER
void machine_test_monitor_run(struct machine_test *emulator, u8 step_by_step, const char * bin, u16 memory_start)
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


void machine_test_run(struct machine_test *emulator, const char * bin, u16 memory_start)
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

u8 machine_test_read(void *emulator_void, u16 address)
{
	struct machine_test *emulator = emulator_void;
	u8 data = 0;

	emulator->cpu.cycles++;
	emulator->cpu.read_write = 0;

	data = memory_read(&(emulator->memory), address);

	#ifdef TESTS_SINGLESTEPTESTS
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = CPU6502_FLAG_READ;
	#endif
	return data;
}

void machine_test_write(void *emulator_void, u16 address, u8 data)
{
	struct machine_test *emulator = emulator_void;

	emulator->cpu.cycles++;
	emulator->cpu.read_write = 1;

	memory_write(&(emulator->memory), address, data);

	#ifdef TESTS_SINGLESTEPTESTS
		c_cycles[c_cycles_i++] = address;
		c_cycles[c_cycles_i++] = data;
		c_cycles[c_cycles_i++] = CPU6502_FLAG_WRITE;
	#else
		if (address == 0xBFFC) {
			emulator->cpu.irq_pin = ((data & 1) > 0) ? CPU6502_IRQ_LOW : CPU6502_IRQ_HIGH;
			emulator->cpu.nmi_pin = (((data >> 1) & 1) > 0) ? CPU6502_NMI_LOW : CPU6502_NMI_HIGH;
		}
	#endif
}
