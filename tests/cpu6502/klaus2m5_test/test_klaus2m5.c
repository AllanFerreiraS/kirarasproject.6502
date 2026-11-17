#include "./../../../emulators/raw/raw.h"
#include "./../../../frontend/disassemble/disassemble.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define TEST_FUNCTIONAL	0
#define	TEST_INTERRUPT	1
#define	TEST_DECIMAL	2

#define PC_ADDR		0x0400
#define MEM_ADDR	0x0000

#define TRUE	1
#define FALSE	0

int main (int argc, const char *argv[])
{
	if (argc <= 1) {
		printf("Error: The path for bin file is required.\n");
		printf("\t>> Example: ./klaus2m5_test ./6502_functional_test.bin\n");
		exit(EXIT_FAILURE);
	}

	const char 		*file_path = argv[1];
	uint16_t 		previous_opcode = 0;
	uint64_t 		counter = 0;
	int8_t 			test_type = -1;
	struct cpu6502 		cpu;
	memory_16		mem;
	disassemble_asm_list 	asm_list;
	infra_raw		emu = {
		.cpu = cpu,
		.memory = mem,
	};


	if (! write_rom(&(emu.memory), file_path, MEM_ADDR) ) {
		printf("Error: file \'%s\' not found.\n", file_path);
		exit(EXIT_FAILURE);
	}

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

	emu.cpu.bus = (buses) { bus_raw_read, bus_raw_write, &emu  };
	cpu6502_init_manual(&(emu.cpu), PC_ADDR);

	printf("Testing...\n");
	while (1) {
		previous_opcode = emu.cpu.reg_pc;
		counter++;

		cpu6502_opcode_execute(&(emu.cpu));

		if (test_type == TEST_INTERRUPT)
			cpu6502_interrupt_handle_request_if_any(&(emu.cpu));

		if (previous_opcode == emu.cpu.reg_pc)
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
	printf("Cycles elapsed:       \t 0x%016lx\n", emu.cpu.cycles);
	printf("Final PC:             \t 0x%04x\n", emu.cpu.reg_pc);
	printf("-------------------------------------------\n");
	printf("RESULT:\n");
	if ((test_type == TEST_FUNCTIONAL) && (emu.cpu.reg_pc == 0x3469)) {
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("\t\t\t success         ;if you get here everything went well\n");
		printf("3469 : 4c6934\t >       jmp *           ;test passed, no errors\n");
		printf("\t\t; S U C C E S S ************************************************\n");
	}
	else if ((test_type == TEST_INTERRUPT) && (emu.cpu.reg_pc == 0x6F5)){
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("4C F5 06\t\t success         ;if you get here everything went well\n");
		printf("\t\t; -------------\n");
		printf("\t\t; S U C C E S S ************************************************\n");
	}
	else if ((test_type == TEST_DECIMAL) && (emu.cpu.reg_pc == 0x044B) && (memory_read(&(emu.memory),0x0B) == 0)) {
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("4C 4B 04\t\t end_of_test      ;jmp *   ;test passed, no errors.\n");
		printf("\t\t; -------------\n");
		printf("\t\t; S U C C E S S ************************************************\n");

	} else {
		printf(">> Your test FAILED. Check the your last OPCODE executed and check it in .lst file\n");
	}

	return 0;
}
