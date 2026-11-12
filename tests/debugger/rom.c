#include "../../emulators/raw/raw.h"
#include <stdio.h>
#include <stdint.h>


int main()
{
	processor_6502 cpu;
	memory_16 mem;

	infra_raw emu = {
		.cpu 		= cpu,
		.memory		= mem,	
	};

	raw_run_with_monitor(&emu, 1, "bins/rom.bin", 0x8000);

	return 0;
}
