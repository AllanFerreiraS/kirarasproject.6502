
// disasm6502 O NOME
#include "./disassemble.h"
#include "./../../include/hardwares/cpu6502.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Buffers
#define STR_OPCODE_ARG_LEN	0x10

// Special OPCODES that needs to be handled separately.
#define OPCODE_BRK		0x00
#define OPCODE_JSR		0x20
#define OPCODE_BCC		0x90
#define OPCODE_BCS		0xB0
#define OPCODE_BEQ		0xF0
#define OPCODE_BNE		0xD0
#define OPCODE_BPL		0x10
#define OPCODE_BMI		0x30
#define OPCODE_BVC		0x50
#define OPCODE_BVS		0x70

/*
 * Todo later:
 * -> Show properly OPCODES that uses the Accumulator Addr Mode
 */


u8 is_opcode_branch(u8 opcode)
{
	if (opcode == OPCODE_BCC)
		return true;
	if (opcode == OPCODE_BCS)
		return true;
	if (opcode == OPCODE_BEQ)
		return true;
	if (opcode == OPCODE_BNE)
		return true;
	if (opcode == OPCODE_BPL)
		return true;
	if (opcode == OPCODE_BMI)
		return true;
	if (opcode == OPCODE_BVC)
		return true;
	if (opcode == OPCODE_BVS)
		return true;
	return false;
}

void get_str_opcode_arg(char *buffer, u8 *rom, u8 addr_mode, u32 *index, u32 last_index)
{
	u16 op_arg;

	for (u8 i = 0; i < STR_OPCODE_ARG_LEN + 1; i++)
		buffer[i] = '\0';

	(*index)++; // OPCODE Read

	switch (addr_mode) {
	case CPU6502_ADDR_MODE_IMMEDIATE_CODE:

		// Special case
		if ( rom[((*index) - 1)] == OPCODE_BRK )
			break;

		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "#$%02x", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_ZERO_PAGE_CODE:
		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "$%02x", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_ZERO_PAGE_X_CODE:
		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "$%02x,X", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_ZERO_PAGE_Y_CODE:
		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "$%02x,Y", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_ABSOLUTE_CODE:
		op_arg = (((u16) rom[(*index) + 1]) << 8) | ((u16) rom[(*index)]);
		(void) sprintf(buffer, "$%04x", op_arg);
		(*index) = (*index) + 2;
		break;
	case CPU6502_ADDR_MODE_ABSOLUTE_X_CODE:
		op_arg = (((u16) rom[(*index) + 1]) << 8) | ((u16) rom[(*index)]);
		(void) sprintf(buffer, "$%04x,X", op_arg);
		(*index) = (*index) + 2;
		break;
	case CPU6502_ADDR_MODE_ABSOLUTE_Y_CODE:
		op_arg = (((u16) rom[(*index) + 1]) << 8) | ((u16) rom[(*index)]);
		(void) sprintf(buffer, "$%04x,Y", op_arg);
		(*index) = (*index) + 2;
		break;
	case CPU6502_ADDR_MODE_INDIRECT_CODE:
		op_arg = (((u16) rom[(*index) + 1]) << 8) | ((u16) rom[(*index)]);
		(void) sprintf(buffer, "($%04x)", op_arg);
		(*index) = (*index) + 2;
		break;
	case CPU6502_ADDR_MODE_INDIRECT_X_CODE:
		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "($%02x,X)", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_INDIRECT_Y_CODE:
		op_arg = (u16) rom[(*index)];
		(void) sprintf(buffer, "($%02x),Y", op_arg);
		(*index)++;
		break;
	case CPU6502_ADDR_MODE_IMPLIED_CODE:
		break;
	case CPU6502_ADDR_MODE_RELATIVE_CODE:

		// Special cases:
		if ( rom[(*index) - 1] == OPCODE_JSR ) {
			op_arg = (((u16) rom[(*index) + 1]) << 8) | ((u16) rom[(*index)]);
			(void) sprintf(buffer, "$%04x", op_arg);
			(*index) = (*index) + 2;
			break;
		}
		// If it is a BRANCH
		if ( is_opcode_branch(rom[(*index) - 1]) ) {
			op_arg = ((u16) last_index + (u16) 2) + (i8) rom[(*index)];
			(void) sprintf(buffer, "$%04x", op_arg);
			(*index)++;
			break;
		}

		(*index)++;
		break;
	case CPU6502_ADDR_MODE_ERROR_CODE:
		break;
	default:
		break;
	}
}

struct disasm_list disasm(u8 *rom, u32 rom_size)
{
	char 		*op_name;
	char 		*mode_name;
	u8		mode_code;
	char 		str_opcode_arg_buffer[STR_OPCODE_ARG_LEN + 1];
	char 		str_assembly[DISASM_STR_OPCODE_LEN + 1];

	u32	index 	 	= 0x0000;
	u32	opcode_index 	= 0x0000;

	struct disasm_list list = { .head = NULL, .tail = NULL };

	while (index < rom_size) {

		opcode_index = index; // Get actual OPCODE position
		(void) cpu6502_opcode_get_infos(rom[opcode_index], &op_name, &mode_name, &mode_code);

		get_str_opcode_arg(str_opcode_arg_buffer, rom, mode_code, &index, opcode_index);
		(void) sprintf(str_assembly, "0x%08x:  %s %s \t;%s", opcode_index, op_name, str_opcode_arg_buffer, mode_name);

		if (! disasm_alloc_code(&list, opcode_index, str_assembly) ) {
			perror("Memory Allocation");
			free(op_name);
			free(mode_name);
			break;
		}
		free(op_name);
		free(mode_name);
	}
	return list;
}


u8 disasm_alloc_code(struct disasm_list *list, u32 index, char *text)
{
	u16 end_str_index = 0;
	struct disasm_code *data = (struct disasm_code *) malloc( sizeof(struct disasm_code) );
	struct disasm_code *previous = NULL;

	if (data == NULL)
		return false;

	for (u16 i = 0; i < DISASM_STR_OPCODE_LEN; i++) {
		data->text[i] = text[i];
		end_str_index = i;
	}
	data->index = index;
	data->text[end_str_index] = '\0';
	data->next = NULL;

	if (list->head == NULL || list->tail == NULL) {
		data->previous	= NULL;
		list->head 	= data;
		list->tail 	= data;
	} else {
		previous		= list->tail;
		list->tail->next 	= data;
		list->tail 		= list->tail->next;
		list->tail->previous	= previous;
	}
	return true;
}

u8 disasm_get_code(struct disasm_list *list, struct disasm_code **explorer, u32 index, u32 *index_chain)
{
	// Reset all infos
	*index_chain 	= 0;
	*explorer	= list->head;

	while (true) {

		if (*explorer == NULL)
			return false;

		if ((*explorer)->index == index)
			return true;
		else
			*explorer = (*explorer)->next;
		(*index_chain)++;
	}
}

u8 disasm_code_next(struct disasm_code **code)
{
	if ((*code)->next == NULL)
		return false;

	*code = (*code)->next;
	return true;
}

u8 disasm_code_previous(struct disasm_code **code)
{
	if ((*code)->previous == NULL)
		return false;

	*code = (*code)->previous;
	return true;
}
