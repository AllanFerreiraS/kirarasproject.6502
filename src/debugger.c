#include "../include/debugger.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

// Terminal Screen Infos
#define SCREEN_HEIGHT		31
#define SCREEN_WIDTH		121
#define SCREEN_MIDDLE_W		(int)(SCREEN_WIDTH/2)
#define SCREEN_MIDDLE_H		(int)(SCREEN_HEIGHT/2)
#define SCREEN_TOP		1
#define SCREEN_BOTTOM		SCREEN_HEIGHT
#define SCREEN_LEFT		1
#define SCREEN_RIGHT		SCREEN_WIDTH
#define HSPLIT_POSITION		SCREEN_MIDDLE_H
#define VSPLIT_POSITION		SCREEN_MIDDLE_W

// Painel Section Infors
#define ASM_COLUM		(VSPLIT_POSITION + 1)
#define ASM_LINE		(SCREEN_TOP + 4)

#define ZERO_PAGE_COLUMN	SCREEN_LEFT
#define ZERO_PAGE_LINE		SCREEN_TOP

#define REGISTERS_COLUMN	(VSPLIT_POSITION + 1)
#define REGISTERS_LINE		SCREEN_TOP

#define CURSOR_REST_LINE	SCREEN_BOTTOM
#define CURSOR_REST_COLUMN	(VSPLIT_POSITION + 1)

#define ROM_COLUM		SCREEN_LEFT
#define ROM_LINE		SCREEN_TOP

// Others
#define PAINEL_DATA_LEN		0x200
#define AMOUNT_PRINT_ASM_LINES	23
#define CURRENT_OPCODE_OFFSET	11

static uint8_t previous_painel_data[PAINEL_DATA_LEN] = {0};


/*
 * LATER CHECKS:
 * -> In debugger_terminal_run
 * -> In drawn_cursor_rest_panel, stop the operation with the stop_flag
 * -> How the drawn_asm_text will behave when OPCODE executed is at the end of ROM?
 */


void update_previous_painel_data(memory_16 * memory)
{
	for (uint16_t i = 0; i < PAINEL_DATA_LEN; i++)
		previous_painel_data[i] = memory_read(memory, i);
}

void drawn_border()
{
	tui_erase_screen(ERASE_ENTIRE_SCREEN);
	tui_move_cursor_home();

	// Drawn vertical split
	tui_move_cursor_coord(1, VSPLIT_POSITION);
	for (uint8_t i = 1; i <= (SCREEN_HEIGHT + 1); i++) {
		printf("|");
		tui_move_cursor(1, MOVE_LEFT);
		tui_move_cursor(1, MOVE_DOWN);
	}
}

void drawn_painel_data(memory_16 * memory)
{
	uint8_t	line_pos = ZERO_PAGE_LINE;
	uint8_t col_pos = ZERO_PAGE_COLUMN + 1;
	uint8_t data = 0;
	
	tui_move_cursor_coord(line_pos, col_pos);
	printf("     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");

	for (uint16_t i = 0x00; i < PAINEL_DATA_LEN; i++) {

		// ADDR
		if ((i % 0x10) == 0) {
			tui_move_cursor_coord(++line_pos, col_pos);
			if (i < 0x100)
				printf(" %02x  ", i);
			else
				printf("%03x  ", i);
		}
		
		// DATA
		data = memory_read(memory, i);
		tui_change_terminal_color(COLOR_FORE_GREEN);
		if (data == previous_painel_data[i]) {
			printf("%02x ", data);
		} else {
			tui_change_terminal_color(COLOR_BACK_WHITE);
			printf("%02x ", data);
			tui_change_terminal_color(COLOR_BACK_DEFAULT);
		}
		tui_change_terminal_color(COLOR_FORE_DEFAULT);
	}
}

void drawn_registers(processor_6502 * processor)
{
	uint8_t line_pos = REGISTERS_LINE;
	uint8_t col_pos = REGISTERS_COLUMN + 1;
	
	// Print the Registers A, X and Y
	tui_move_cursor_coord(line_pos, col_pos);
	printf("A:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%02x", processor->reg_a);
	tui_terminal_color_default();
	printf("  X:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%02x", processor->reg_x);
	tui_terminal_color_default();
	printf("  Y:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%02x", processor->reg_y);
	tui_terminal_color_default();

	// Print the FLAG and CYCLES
	debugger_print_formated_flags("  FLAG", processor->reg_p);
	printf("  CYCLES:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%016lx", processor->cycles);
	tui_terminal_color_default();

	// Print the Interrupts
	tui_move_cursor_coord(++line_pos, col_pos);
	printf("IRQ=");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%s", (processor->irq_pin == IRQ_LOW_VALUE) ? "LOW" : "HIGH");
	tui_terminal_color_default();
	printf("  NMI=");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("%s", (processor->nmi_pin == NMI_LOW_VALUE) ? "LOW" : "HIGH");
	tui_terminal_color_default();
	printf("  |->  REQUESTS: ");
	if (processor->irq_pin == IRQ_LOW_VALUE) {
		tui_change_terminal_color(COLOR_BACK_GREEN);
		printf("IRQ");
		tui_terminal_color_default();
	} else {
		printf("IRQ");
	}
	printf("  ");
	if (processor->nmi_request) {
		tui_change_terminal_color(COLOR_BACK_GREEN);
		printf("NMI");
		tui_terminal_color_default();
	} else {
		printf("NMI");
	}

	// Print the PC and Stack value 
	tui_move_cursor_coord(++line_pos,col_pos);
	printf("PC:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("0x%04x", processor->reg_pc);
	tui_terminal_color_default();
	printf("  STACK:");
	tui_change_terminal_color(COLOR_FORE_GREEN);
	printf("0x%03x", processor->reg_sp);
	tui_terminal_color_default();
}

void drawn_cursor_rest_panel(uint8_t stop)
{
	uint8_t line_pos = CURSOR_REST_LINE;
	uint8_t col_pos = CURSOR_REST_COLUMN + 1;
	char stop_flag;

	tui_move_cursor_coord(line_pos, col_pos);
	tui_colorful_print("Press any key to continue. (\'q\' to exit): ",COLOR_BACK_BLUE);
	if (stop)
		stop_flag = getchar();
}

void drawn_asm_text(disassemble_asm_list *asm_list, uint64_t index)
{
	uint8_t line_pos	= ASM_LINE;
	uint8_t col_pos 	= ASM_COLUM + 1;

	uint8_t empty_lines	= CURRENT_OPCODE_OFFSET;
	uint64_t index_chain_ignore = 0;
	disassemble_asm_code *code;

	if (! disassemble_get_asm_code(asm_list, &code, index, &index_chain_ignore) ) {
		printf("Error printing assembly code");
		return;
	}

	for (uint8_t i = 0; i < CURRENT_OPCODE_OFFSET; i++) {
		if (! disassemble_asm_code_previous(&code) )
			break;
		empty_lines--;
	}

	tui_move_cursor_coord(line_pos, col_pos);	
	for (uint8_t i = 0; i < AMOUNT_PRINT_ASM_LINES; i++) {
		
		if (i < empty_lines) {
			printf(" ");
			tui_move_cursor_coord(++line_pos, col_pos);
			continue;
		}
		
		if ( i == CURRENT_OPCODE_OFFSET) {
			tui_change_terminal_color(COLOR_BACK_GREEN);
			printf("%s", code->text);
			tui_terminal_color_default();
		} else {
			printf("%s", code->text);
		}
		disassemble_asm_code_next(&code);
		tui_move_cursor_coord(++line_pos, col_pos);
	}
}




// PUBLIC FUNCTIONS

void debugger_print_formated_flags(const char *label, uint8_t flag)
{
	char *formated = "NV1BDIZC";
	uint8_t mask = (1) << 7;
	
	printf("%s:", label);
	for (uint8_t i = 0; i < 8; i++) {
		if ((flag & mask) > 0) {
			tui_change_terminal_color(COLOR_FORE_GREEN);
			printf("%c", formated[i]);
			tui_terminal_color_default();
		} else {
			printf("%c", formated[i]);
		}
		mask = mask >> 1;
	}
}

void debugger_terminal_run(processor_6502 * processor, memory_16 * memory, disassemble_asm_list *asm_list, uint8_t stop)
{
	drawn_border();

	// LEMBRANDO QUE VAI DEPENDER DO MAPPER DEPOIS DE SER IMPLEMENTADO
	drawn_asm_text(asm_list, processor->reg_pc);

	drawn_painel_data(memory);
	drawn_registers(processor);
	drawn_cursor_rest_panel(stop);

	// Update
	update_previous_painel_data(memory);
	fflush(stdout);

	if (! stop)
		usleep(13 * 1000);
}

void debugger_terminal_prepare()
{
	tui_terminal_init();
}

void debugger_terminal_finish()
{
	tui_terminal_cleanup();
}

