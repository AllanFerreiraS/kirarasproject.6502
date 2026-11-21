#include "./../../../emulators/machine_test/machine_test.h"
#include "./../../../frontend/disassemble/disassemble.h"
#include "./../../../include/utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define KLAUS_LABEL_ERROR	"[Klaus2m5 Test Error]:"

#define TEST_FUNCTIONAL		0
#define	TEST_INTERRUPT		1
#define	TEST_DECIMAL		2

#define PC_ADDR			0x0400
#define MEM_ADDR		0x0000

int main (int argc, const char *argv[])
{
	if (argc <= 1) {
		printf("Error: The path for bin file is required.\n");
		printf("\t>> Example: ./klaus2m5_test ./6502_functional_test.bin\n");
		exit(EXIT_FAILURE);
	}

	const char 		*file_path = argv[1];
	u16			previous_opcode = 0;
	u64			counter = 0;
	i8 			test_type = -1;
	struct cpu6502 		cpu;
	struct memory		mem;
	struct machine_test	emulator = { .cpu = cpu, .ram = mem };
	enum   memory_errno ramstatus;
	enum   cpu6502_errno cpustatus;

	if ( strstr(file_path, "6502_functional_test.bin") != NULL)
		test_type = TEST_FUNCTIONAL;
	else if ( strstr(file_path, "6502_interrupt_test.bin") != NULL)
		test_type = TEST_INTERRUPT;
	else if ( strstr(file_path, "6502_decimal_test.bin") != NULL)
		test_type = TEST_DECIMAL;

	if (test_type == -1) {
		printf("Error: \'%s\' is not a know bin file name to klaus2m5 test.\n", file_path);
		printf("\t>> Please, try to use a correct bin file, or rebuild the file using these names:\n");
		printf("\t\t- 6502_functional_test.bin\n");
		printf("\t\t- 6502_interrupt_test.bin\n");
		printf("\t\t- 6502_decimal_test.bin\n");
		exit(EXIT_FAILURE);
	}

	ramstatus = machine_test_prepare_memory(&emulator, file_path, MEM_ADDR);
	if (ramstatus == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "%s Memory allocation failed.\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "%s Wrong Memory Type (ROM or RAM).\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (ramstatus == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "%s Wrong Memory Target (PRG or CHR).\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (ramstatus == MEMORY_ERRNO_SEGFAULT) {
		fprintf(stderr, "%s SEGFAULT.\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (ramstatus == MEMORY_ERRNO_FILE_NULL) {
		fprintf(stderr, "%s Error opening rom file\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	cpustatus = machine_test_prepare_cpu6502(&emulator);
	if (cpustatus == CPU6502_ERRNO_BUS_FUNCTION_NULL) {
		fprintf(stderr, "%s Bus Read and/or Bus Write pointers are NULL\n", KLAUS_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	cpu6502_init_manual(&(emulator.cpu), PC_ADDR);
	printf("Testing...\n");
	while (1) {
		previous_opcode = emulator.cpu.reg_pc;
		counter++;

		cpu6502_opcode_execute(&(emulator.cpu));

		if (test_type == TEST_INTERRUPT)
			cpu6502_interrupt_handle_request_if_any(&(emulator.cpu));

		if (previous_opcode == emulator.cpu.reg_pc)
			break;
	}

	if (test_type == TEST_FUNCTIONAL)
		printf("Klaus2m5 6502 Functional Test\n");
	else if (test_type == TEST_INTERRUPT)
		printf("Klaus2m5 6502 Interrupt Test\n");
	else if (test_type == TEST_DECIMAL)
		printf("Klaus2m5 6502 Decimal Test\n");
	else
		printf("WTF Test\n");

	printf("-------------------------------------------\n");
	printf("Instructions executed:\t %ld\n", counter);
	printf("Cycles elapsed:       \t 0x%016lx\n", emulator.cpu.cycles);
	printf("Final PC:             \t 0x%04x\n", emulator.cpu.reg_pc);
	printf("-------------------------------------------\n");
	printf("RESULT:\n");
	if ((test_type == TEST_FUNCTIONAL) && (emulator.cpu.reg_pc == 0x3469)) {
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("\t\t\t success         ;if you get here everything went well\n");
		printf("3469 : 4c6934\t >       jmp *           ;test passed, no errors\n");
		printf("\t\t; S U C C E S S ************************************************\n");
	}
	else if ((test_type == TEST_INTERRUPT) && (emulator.cpu.reg_pc == 0x6F5)){
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("4C F5 06\t\t success         ;if you get here everything went well\n");
		printf("\t\t; -------------\n");
		printf("\t\t; S U C C E S S ************************************************\n");
	}
	else if ((test_type == TEST_DECIMAL) && (emulator.cpu.reg_pc == 0x044B) && (memory_read(&(emulator.ram),0x0B) == 0)) {
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("4C 4B 04\t\t end_of_test      ;jmp *   ;test passed, no errors.\n");
		printf("\t\t; -------------\n");
		printf("\t\t; S U C C E S S ************************************************\n");

	} else {
		printf(">> Your test FAILED. Check the your last OPCODE executed and check it in .lst file\n");
	}

	memory_free(&(emulator.ram));

	return 0;
}
