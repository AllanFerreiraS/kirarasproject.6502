#include "./../../../emulators/machine_test/machine_test.h"
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
	struct cpu6502 	cpu;
	memory_16 	mem;

	struct machine_test emulator = {
		.cpu 		= cpu,
		.memory		= mem,
	};

	machine_test_monitor_run(&emulator, 0,file_test, 0x0000);

	return 0;
}
