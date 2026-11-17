#ifndef _MACHINE_TEST_H_
#define _MACHINE_TEST_H_

#include "./../../include/hardwares/cpu6502.h"
#include "./../../include/hardwares/memory_16.h"
#include "./../../include/buses.h"
#include "./../../include/utils.h"

struct machine_test {
	struct cpu6502 	cpu;
	memory_16	memory;
};

#ifdef FRONTEND_DEBUGGER
	#include "./../../frontend/debugger/debugger.h"
	void 	machine_test_monitor_run(struct machine_test *emulator, u8 step_by_step, const char * bin, u16 memory_start);
#endif

u8	machine_test_prepare_rom(struct machine_test *emulator, const char *bin, u16 memory_start);
void	machine_test_prepare_cpu(struct machine_test *emulator);
u8	machine_test_read(void *emulator_void, u16 address);
void 	machine_test_write(void *emulator_void, u16 address, u8 data);
void 	machine_test_run(struct machine_test *emulator, const char * bin, u16 memory_start);

#endif
