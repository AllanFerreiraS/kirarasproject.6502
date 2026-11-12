#include "../include/disassemble.h"
#include "../include/hardwares/processor_6502.h"
#include <stdio.h>
#include <stdlib.h>
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

#define TRUE			1
#define FALSE			0


uint8_t is_opcode_branch(uint8_t opcode)
{
	if (opcode == OPCODE_BCC)
		return TRUE;
	if (opcode == OPCODE_BCS)
		return TRUE;
	if (opcode == OPCODE_BEQ)
		return TRUE;
	if (opcode == OPCODE_BNE)
		return TRUE;
	if (opcode == OPCODE_BPL)
		return TRUE;
	if (opcode == OPCODE_BMI)
		return TRUE;
	if (opcode == OPCODE_BVC)
		return TRUE;
	if (opcode == OPCODE_BVS)
		return TRUE;
	return FALSE;
}

void get_str_opcode_arg(char *buffer, uint8_t *rom, uint8_t addr_mode, uint64_t *index, uint64_t last_index)
{
	uint16_t op_arg;

	for (uint8_t i = 0; i < STR_OPCODE_ARG_LEN + 1; i++)
		buffer[i] = '\0';

	(*index)++; // OPCODE Read

	switch (addr_mode) {
	case PROCESSOR_6502_ADDR_MODE_IMMEDIATE_CODE:
		
		// Special case
		if ( rom[((*index) - 1)] == OPCODE_BRK )
			break;

		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "#$%02x", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_CODE:
		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "$%02x", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_X_CODE:
		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "$%02x,X", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_Y_CODE:
		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "$%02x,Y", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_ABSOLUTE_CODE:
		op_arg = (((uint16_t) rom[(*index) + 1]) << 8) | ((uint16_t) rom[(*index)]);
		(void) sprintf(buffer, "$%04x", op_arg);
		(*index) = (*index) + 2;
		break;
	case PROCESSOR_6502_ADDR_MODE_ABSOLUTE_X_CODE:
		op_arg = (((uint16_t) rom[(*index) + 1]) << 8) | ((uint16_t) rom[(*index)]);
		(void) sprintf(buffer, "$%04x,X", op_arg);
		(*index) = (*index) + 2;
		break;
	case PROCESSOR_6502_ADDR_MODE_ABSOLUTE_Y_CODE:
		op_arg = (((uint16_t) rom[(*index) + 1]) << 8) | ((uint16_t) rom[(*index)]);
		(void) sprintf(buffer, "$%04x,Y", op_arg);
		(*index) = (*index) + 2;
		break;
	case PROCESSOR_6502_ADDR_MODE_INDIRECT_CODE:
		op_arg = (((uint16_t) rom[(*index) + 1]) << 8) | ((uint16_t) rom[(*index)]);
		(void) sprintf(buffer, "($%04x)", op_arg);
		(*index) = (*index) + 2;
		break;
	case PROCESSOR_6502_ADDR_MODE_INDIRECT_X_CODE:
		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "($%02x,X)", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_INDIRECT_Y_CODE:
		op_arg = (uint16_t) rom[(*index)];
		(void) sprintf(buffer, "($%02x),Y", op_arg);
		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_IMPLIED_CODE:
		break;
	case PROCESSOR_6502_ADDR_MODE_RELATIVE_CODE:

		// Special cases:
		if ( rom[(*index) - 1] == OPCODE_JSR ) {
			op_arg = (((uint16_t) rom[(*index) + 1]) << 8) | ((uint16_t) rom[(*index)]);
			(void) sprintf(buffer, "$%04x", op_arg);
			(*index) = (*index) + 2;
			break;
		}
		// If it is a BRANCH	
		if ( is_opcode_branch(rom[(*index) - 1]) ) {
			op_arg = ((uint16_t) last_index + (uint16_t) 2) + (int8_t) rom[(*index)];
			(void) sprintf(buffer, "$%04x", op_arg);
			(*index)++;
			break;
		}

		(*index)++;
		break;
	case PROCESSOR_6502_ADDR_MODE_ERROR_CODE:
		break;
	default:
		break;
	}
}

disassemble_asm_list disassemble(uint8_t *rom, uint64_t rom_size)
{
	char 		*op_name;
	char 		*mode_name;
	uint8_t		mode_code;
	char 		str_opcode_arg_buffer[STR_OPCODE_ARG_LEN + 1];
	char 		str_assembly[DISASSEMBLE_STR_OPCODE_LEN + 1];

	uint64_t	index 	 	= 0x0000;
	uint64_t	opcode_index 	= 0x0000;
	
	disassemble_asm_list list = { .head = NULL, .tail = NULL };
	
	while (index < rom_size) {

		opcode_index = index; // Get actual OPCODE position
		(void) processor_6502_get_opcode_infos(rom[opcode_index], &op_name, &mode_name, &mode_code);
		
		get_str_opcode_arg(str_opcode_arg_buffer, rom, mode_code, &index, opcode_index);
		(void) sprintf(str_assembly, "0x%08lx:  %s %s \t;%s", opcode_index, op_name, str_opcode_arg_buffer, mode_name);

		if (! disassemble_alloc_asm_code(&list, opcode_index, str_assembly) ) {
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


uint8_t disassemble_alloc_asm_code(disassemble_asm_list *list, uint64_t index, char *text)
{
	uint16_t end_str_index = 0;
	
	disassemble_asm_code *data = (disassemble_asm_code *) malloc( sizeof(disassemble_asm_code) );

	if (data == NULL)
		return FALSE;

	for (uint16_t i = 0; i < DISASSEMBLE_STR_OPCODE_LEN; i++) {
		data->text[i] = text[i];
		end_str_index = i;
	}
	data->index = index;
	data->text[end_str_index] = '\0';
	data->next = NULL;

	if (list->head == NULL || list->tail == NULL) {
		list->head = data;
		list->tail = data;
	} else {
		list->tail->next = data;
		list->tail = list->tail->next;
	}
	return TRUE;
}

uint8_t disassemble_get_asm_code(disassemble_asm_code **explorer, uint64_t index, uint64_t *index_chain)
{
	*index_chain 	= 0;

	while (TRUE) {

		if (*explorer == NULL)
			return FALSE;
		
		if ((*explorer)->index == index)
			return TRUE;
		else
			*explorer = (*explorer)->next;
		
		(*index_chain)++;
	}
}
