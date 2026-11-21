#ifndef _MACHINE_TEST_H_
#define _MACHINE_TEST_H_

#include "./../../include/hardwares/cpu6502.h"
#include "./../../include/hardwares/memory.h"
#include "./../../include/buses.h"
#include "./../../include/utils.h"

struct machine_test {
	struct cpu6502 	cpu;
	struct memory	ram;
};

#ifdef FRONTEND_DEBUGGER
	#include "./../../frontend/debugger/debugger.h"
	u8 	machine_test_monitor_run(struct machine_test *emulator, u8 step_by_step, const char *rom, u16 memory_start);
#endif

enum memory_errno machine_test_prepare_memory(struct machine_test *emulator, const char *rom, u16 memory_start);
enum cpu6502_errno machine_test_prepare_cpu6502(struct machine_test *emulator);

u8 	machine_test_run(struct machine_test *emulator, const char *rom, u16 memory_start);
u8	machine_test_read(void *emulator_void, u16 address);
void 	machine_test_write(void *emulator_void, u16 address, u8 data);

#endif
