#include "./../../../emulators/machine_test/machine_test.h"
#include <stdio.h>

#define TEST_DEBUGGER_LABEL	"[Debugger Test]:"

int main(int argc, const char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "%s The path for bin file is required.\n", TEST_DEBUGGER_LABEL);
		fprintf(stderr, "\t>> Example: ./test_debugger ./rom_1.bin\n");
		exit(EXIT_FAILURE);
	}

	const char *file_test = argv[1];
	struct cpu6502 	cpu;
	struct memory 	mem;

	struct machine_test emulator = {
		.cpu 		= cpu,
		.ram		= mem,
	};

	machine_test_monitor_run(&emulator, 0,file_test, 0x0000);

	return 0;
}
