#include "../../../emulators/raw/raw.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, const char *argv[])
{

	if (argc <= 1) {
		printf("Error: The path for bin file is required.\n");
		printf("\t>> Example: ./test_debugger ./rom_1.bin\n");
		exit(EXIT_FAILURE);
	}
	
	const char	*file_test = argv[1];
	processor_6502 	cpu;
	memory_16 	mem;

	infra_raw emu = {
		.cpu 		= cpu,
		.memory		= mem,	
	};

	raw_run_with_monitor(&emu, 1, file_test, 0x0000);

	return 0;
}
