#include "./machine_test.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MACHINE_TEST_LABEL_ERROR	"[Machine_Test Error]:"

// Used for SingleStepTest Only. TESTS_SINGLESTEPTESTS should be defined.
u16 c_cycles[3 * 256] = {0};
u16 c_cycles_i = 0;


enum memory_errno machine_test_prepare_memory(struct machine_test *emulator, const char *rom, u16 memory_start)
{
	enum memory_errno ramstatus;
	ramstatus = memory_alloc(&(emulator->ram), MEMORY_TYPE_RAM, MEMORY_TARGET_PGR, U16_SIZE);
	if (ramstatus != MEMORY_ERRNO_OK)
		return ramstatus;

	ramstatus = memory_rom_burn(&(emulator->ram), rom, memory_start);
	return ramstatus;
}

enum cpu6502_errno machine_test_prepare_cpu6502(struct machine_test *emulator)
{
	enum cpu6502_errno cpustatus;
	cpustatus = cpu6502_bus_prepare(&(emulator->cpu), machine_test_read, machine_test_write, emulator);
	return cpustatus;
}



#ifdef FRONTEND_DEBUGGER
u8 machine_test_monitor_run(struct machine_test *emulator, u8 step_by_step, const char *rom, u16 memory_start)
{
	u8 return_status = true;
	enum memory_errno ramstatus;
	struct disasm_list asm_list;

	ramstatus = machine_test_prepare_memory(emulator, rom, memory_start);
	if (ramstatus == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "%s Memory allocation failed.\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "%s Wrong Memory Type (ROM or RAM).\n", MACHINE_TEST_LABEL_ERROR);
		return_status =  false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "%s Wrong Memory Target (PRG or CHR).\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_SEGFAULT) {
		fprintf(stderr, "%s SEGFAULT.\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_FILE_NULL) {
		fprintf(stderr, "%s Error opening rom file\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	}

	enum cpu6502_errno cpustatus;
	cpustatus = machine_test_prepare_cpu6502(emulator);
	if (cpustatus == CPU6502_ERRNO_BUS_FUNCTION_NULL) {
		fprintf(stderr, "%s Bus Read and/or Bus Write pointers are NULL\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	}


	asm_list = disasm(emulator->ram.data, emulator->ram.size);

	if (asm_list.head == NULL) {
		fprintf(stderr, "%s asm_list is null.\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;

	}

	cpu6502_init(&(emulator->cpu), CPU6502_POWER_UP);
	debugger_terminal_prepare();
	debugger_terminal_run(&(emulator->cpu), &(emulator->ram), &asm_list, step_by_step);
	while(1) {

		cpu6502_opcode_execute(&(emulator->cpu));
		debugger_terminal_run(&(emulator->cpu), &(emulator->ram), &asm_list, step_by_step);
		cpu6502_interrupt_handle_request_if_any(&(emulator->cpu));
	}
	debugger_terminal_finish();

stop_running:
	memory_free(&(emulator->ram));
	return return_status;
}
#endif

u8 machine_test_run(struct machine_test *emulator, const char *rom, u16 memory_start)
{
	u8 return_status = true;

	enum memory_errno ramstatus;
	ramstatus = machine_test_prepare_memory(emulator, rom, memory_start);
	if (ramstatus == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "%s Memory allocation failed.\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "%s Wrong Memory Type (ROM or RAM).\n", MACHINE_TEST_LABEL_ERROR);
		return_status =  false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "%s Wrong Memory Target (PRG or CHR).\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_SEGFAULT) {
		fprintf(stderr, "%s SEGFAULT.\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	} else if (ramstatus == MEMORY_ERRNO_FILE_NULL) {
		fprintf(stderr, "%s Error opening rom file\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	}

	enum cpu6502_errno cpustatus;
	cpustatus = machine_test_prepare_cpu6502(emulator);
	if (cpustatus == CPU6502_ERRNO_BUS_FUNCTION_NULL) {
		fprintf(stderr, "%s Bus Read and/or Bus Write pointers are NULL\n", MACHINE_TEST_LABEL_ERROR);
		return_status = false;
		goto stop_running;
	}

	cpu6502_init(&(emulator->cpu), CPU6502_POWER_UP);
	while(1) {
		cpu6502_opcode_execute(&(emulator->cpu));
		cpu6502_interrupt_handle_request_if_any(&(emulator->cpu));
	}

stop_running:
	memory_free(&(emulator->ram));
	return return_status;
}

u8 machine_test_read(void *emulator_void, u16 address)
{
	struct machine_test *emulator = emulator_void;
	u8 data = 0;

	data = memory_read(&(emulator->ram), address);

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

	memory_write(&(emulator->ram), address, data);

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
