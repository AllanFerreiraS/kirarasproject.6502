//#include "./../../emulators/raw/raw.h"
#include "./../../include/hardwares/processor_6502.h"
#include "./../../include/hardwares/memory_16.h"
#include "./../../include/disassemble.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define	PC_ADDR		0x0400
#define	MEM_ADDR	0x8000
#define	KLAUS_BIN	"disasm_test.bin"

int main()
{
	memory_16		mem;
	disassemble_asm_list	asm_list;
	disassemble_asm_code	*asm_code;
	uint64_t index_found = 0;

	if (! write_rom(&mem, KLAUS_BIN, MEM_ADDR) ) {
		printf("Error: \'%s\' not found.\n", KLAUS_BIN);
		exit(EXIT_FAILURE);
	}

	asm_list = disassemble(mem.data, MEMORY_16_SIZE);
	asm_code = asm_list.head;

	if (! disassemble_get_asm_code(&asm_code, 0x8000, &index_found) ) {
		printf("Error getting the tail index\n");
		exit(EXIT_FAILURE);
	}

	while (asm_code != NULL) {
		printf("%s\n", asm_code->text);
		asm_code = asm_code->next;
		usleep(15 * 10000);
	}
}
