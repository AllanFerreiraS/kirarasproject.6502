#include "./../../../frontend/disassemble/disassemble.h"
#include "./../../../include/hardwares/memory_16.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define	MEM_ADDR	0x8000
#define OPCODE_INDEX	0x8000

int main(int argc, const char *argv[])
{
	const char 		*file_test;
	memory_16		mem;
	disassemble_asm_list	asm_list;
	disassemble_asm_code	*asm_code;
	uint64_t 		index_current_opcode = 0;

	if (argc <= 1) {
		printf("Error: The path for the bin file is required.\n");
		printf("\t>> Example: ./test_disasm ./disasm.bin\n");
		exit(EXIT_FAILURE);
	}

	file_test = argv[1];

	// Write in the ROM
	if (! write_rom(&mem, file_test, MEM_ADDR) ) {
		printf("Error: \'%s\' not found.\n", file_test);
		exit(EXIT_FAILURE);
	}

	// Get the disassembled ROM
	asm_list = disassemble(mem.data, MEMORY_16_SIZE);

	// Point to the start of the disassembled ROM
	asm_code = asm_list.head;

	// Make asm_code point to the code where OPCODE points to 0x8000
	if (! disassemble_get_asm_code(&asm_list, &asm_code, OPCODE_INDEX, &index_current_opcode) ) {
		perror("OPCODE in the chosen index not found");
		exit(EXIT_FAILURE);
	}

	for (uint8_t i = 0; i < 15; i++) {
		if (! disassemble_asm_code_previous(&asm_code) ) {
			// asm_code still points to the head
			perror("Tried to go beyond the head limit");
			break;
		}
	}

	// Print the disassembled code
	while (1) {
		printf("%s\n", asm_code->text);
		if (! disassemble_asm_code_next(&asm_code) ) {
			// asm_code still points to the tail
			perror("Tried to go beyond the tail limit");
			printf("EOF\n");
			break;
		}
		usleep(50 * 1000);
	}

	return 0;
}
