#include "./../../../frontend/disassemble/disassemble.h"
#include "./../../../include/hardwares/memory.h"
#include <stdio.h>
#include <unistd.h>

#define DISASM_TEST_LABEL_ERROR	"[Disassemble Test Error]:"
#define DISASM_TEST_LABEL	"[Disassemble Test]:"
#define	MEM_ADDR			0x8000
#define OPCODE_INDEX			0x8000

int main(int argc, const char *argv[])
{
	const char 		*file_test;
	struct memory		mem;
	enum   memory_errno	memorystatus;
	struct disasm_list	asm_list;
	struct disasm_code	*asm_code;
	u32 		index_current_opcode = 0;

	if (argc <= 1) {
		fprintf(stderr, "%s The path for the bin file is required.\n", DISASM_TEST_LABEL_ERROR);
		fprintf(stderr, "\t>> Example: ./test_disasm ./disasm.bin\n");
		exit(EXIT_FAILURE);
	}

	file_test = argv[1];

	memorystatus =  memory_alloc(&mem, MEMORY_TYPE_ROM, MEMORY_TARGET_PGR, U16_SIZE);
	if (memorystatus == MEMORY_ERRNO_ALLOC_FAILED) {
		fprintf(stderr, "%s Memory allocation failed.\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (memorystatus == MEMORY_ERRNO_WRONG_TYPE) {
		fprintf(stderr, "%s Wrong Memory Type (ROM or RAM).\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (memorystatus == MEMORY_ERRNO_WRONG_TARGET) {
		fprintf(stderr, "%s Wrong Memory Target (PRG or CHR).\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (memorystatus == MEMORY_ERRNO_SEGFAULT) {
		fprintf(stderr, "%s SEGFAULT.\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	} else if (memorystatus == MEMORY_ERRNO_FILE_NULL) {
		fprintf(stderr, "%s Error opening rom file\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	memorystatus =  memory_rom_burn(&mem, file_test, MEM_ADDR);
	if (memorystatus != MEMORY_ERRNO_OK) {
		fprintf(stderr, "%s memory_rom_burn() -> File \'%s\' not found.\n", DISASM_TEST_LABEL_ERROR, file_test);
		return false;
	}


	// Get the disassembled ROM
	asm_list = disasm(mem.data, mem.size);

	// Point to the start of the disassembled ROM
	asm_code = asm_list.head;

	// Make asm_code point to the code where OPCODE points to 0x8000
	if (! disasm_get_code(&asm_list, &asm_code, OPCODE_INDEX, &index_current_opcode) ) {
		fprintf(stderr, "%s OPCODE in the chosen index not found\n", DISASM_TEST_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	for (u8 i = 0; i < 15; i++) {
		if (! disasm_code_previous(&asm_code) ) {
			// asm_code still points to the head
			printf("%s Tried to go beyond the head limit\n", DISASM_TEST_LABEL);
			break;
		}
	}

	// Print the disassembled code
	while (1) {
		printf("%s\n", asm_code->text);
		if (! disasm_code_next(&asm_code) ) {
			// asm_code still points to the tail
			printf("%s Tried to go beyond the tail limit\n", DISASM_TEST_LABEL);
			printf("%s EOF\n", DISASM_TEST_LABEL);
			break;
		}
		usleep(50 * 1000);
	}

	memory_free(&mem);
	return 0;
}
