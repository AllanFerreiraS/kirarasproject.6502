#ifndef _DISASSEMBLE_H_
#define _DISASSEMBLE_H_

#include <stdint.h>

#define DISASSEMBLE_STR_OPCODE_LEN	0x100

typedef struct str_disassemble_asm_code {
	uint64_t			index;
	char 				text[DISASSEMBLE_STR_OPCODE_LEN + 1];
	struct str_disassemble_asm_code	*next;
} disassemble_asm_code;

typedef struct {
	disassemble_asm_code *head;
	disassemble_asm_code *tail;
} disassemble_asm_list;

uint8_t disassemble_alloc_asm_code(disassemble_asm_list *list, uint64_t index, char *text);
uint8_t disassemble_get_asm_code(disassemble_asm_code **explorer, uint64_t index, uint64_t *index_chain);
disassemble_asm_list disassemble(uint8_t *rom, uint64_t rom_size);

#endif
