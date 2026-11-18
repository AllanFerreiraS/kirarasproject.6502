#ifndef _NES_H_
#define _NES_H_

#include "./../../include/hardwares/cpu6502.h"
#include "./../../include/hardwares/memory_16.h"
#include "./../../include/buses.h"
#include "./../../include/utils.h"

struct nes {
	struct cpu6502 	cpu;
	memory_16	memory;
};
/*
#ifdef FRONTEND_DEBUGGER
	#include "./../../frontend/debugger/debugger.h"
	void nes_monitor_run(struct nes *emulator, u8 step_by_step, const char * bin, u16 memory_start);
#endif
*/

u8	nes_prepare_rom(struct nes *emulator, const char *bin, u16 memory_start);
void	nes_prepare_cpu(struct nes *emulator);
u8	nes_read(void *emulator_void, u16 address);
void 	nes_write(void *emulator_void, u16 address, u8 data);
void 	nes_run(struct nes *emulator, const char * bin, u16 memory_start);

#endif
