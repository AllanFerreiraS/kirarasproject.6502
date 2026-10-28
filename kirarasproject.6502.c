#include "./include/infrastructure.h"
#include "./include/hardwares/processor_6502.h"
#include "./include/hardwares/memory_16.h"
#include "./emulators/raw/raw.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	processor_6502 	c;
	memory_16	m;

	infra_raw emu_raw = {
		.cpu	= c,
		.memory	= m
	};

raw_run(&emu_raw);

	return EXIT_SUCCESS;
}
