#ifndef _DISASSEMBLE_H_
#define _DISASSEMBLE_H_

#include "./../../include/utils.h"

#define DISASM_STR_OPCODE_LEN	0x100

struct disasm_code {
	u32 index;
	char 	 text[DISASM_STR_OPCODE_LEN + 1];
	struct disasm_code *previous;
	struct disasm_code *next;
};

struct disasm_list{
	struct disasm_code *head;
	struct disasm_code *tail;
};

u8 disasm_alloc_code(struct disasm_list *list, u32 index, char *text);
u8 disasm_get_code(struct disasm_list *list, struct disasm_code **explorer, u32 index, u32 *index_chain);
u8 disasm_code_next(struct disasm_code **code);
u8 disasm_code_previous(struct disasm_code **code);
struct disasm_list disasm(u8 *rom, u32 rom_size);

#endif
