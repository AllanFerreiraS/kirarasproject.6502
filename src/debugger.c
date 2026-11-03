#include "../include/debugger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SCREEN_HEIGHT		31
#define SCREEN_WIDTH		121
#define SCREEN_MIDDLE_W		(int)(SCREEN_WIDTH/2)
#define SCREEN_MIDDLE_H		(int)(SCREEN_HEIGHT/2)
#define HSPLIT_POSITION		(SCREEN_MIDDLE_H + 10)
#define VSPLIT_POSITION		SCREEN_MIDDLE_W

#define ROM_COLUM		1
#define ROM_LINE		1
#define ZERO_PAGE_COLUMN	(VSPLIT_POSITION + 1)
#define ZERO_PAGE_LINE		1
#define REGISTERS_COLUMN	1
#define REGISTERS_LINE		(HSPLIT_POSITION + 1)
#define DOWN_RIGHT_PANEL_LINE	(HSPLIT_POSITION + 1)
#define DOWN_RIGHT_PANEL_COLUMN (VSPLIT_POSITION + 1)

static uint8_t previous_zero_page[0x100] = {0};

void u8_decimal_to_binary_string_(uint8_t v, char * b)
{
	uint8_t i = 0;
	uint8_t mask = 1;
	for (; i < 8; i++) {
		b[i] = ( (v & (mask << (7-i))) > 0 ) ? '1' : '0';
	}
	b[i] = '\0';

}

void show_current_opcode(memory_16 * memory, uint16_t reg_pc)
{
	uint8_t opcode = memory_read(memory, reg_pc);
	char *opcode_name = NULL;
	char *addr_mode_name = NULL;

	get_opcode_infos(opcode, &opcode_name, &addr_mode_name);
	if (opcode_name  == NULL) {
		tui_change_terminal_color(COLOR_BACK_RED);
		printf("OPCODE: <UNKOWN>");
		tui_terminal_color_default();
	} else {
		tui_change_terminal_color(COLOR_FORE_GREEN);
		printf("OPCODE: %s, %s", opcode_name, addr_mode_name);
		tui_terminal_color_default();
	}

	free(opcode_name);
	free(addr_mode_name);
}

void show_formated_flags(uint8_t flag)
{
	char *formated = "NV1BDIZC";
	uint8_t mask = (1) << 7;

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

void update_previous_zero_page(memory_16 * memory)
{
	for (uint16_t i = 0; i < 0x100; i++)
		previous_zero_page[i] = memory_read(memory, i);
}

void drawn_rom(memory_16 * memory, uint16_t reg_pc)
{
	uint8_t line_pos = ROM_LINE;
	uint8_t col_pos = ROM_COLUM + 1;
	uint32_t current_page = reg_pc & 0xFF00;

	tui_move_cursor_coord(line_pos, col_pos);
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-= ROM -=-=-=-=-=-=-=-=-=-=-=-=-");
	tui_move_cursor_coord(++line_pos, col_pos);
	tui_move_cursor_coord(++line_pos, col_pos);
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	tui_move_cursor_coord(++line_pos, col_pos);
	for (uint32_t i = current_page; i < (current_page + 0x100); i++) {
		if ((i % 0x10) == 0) {
			tui_move_cursor_coord(++line_pos, col_pos);
			printf("%02x  ", i);
		}

		if (i == reg_pc) {
			tui_change_terminal_color(COLOR_BACK_RED);
			printf("%02x ", memory->data[i]);
			tui_terminal_color_default();
		} else {
			printf("%02x ", memory->data[i]);
		}
	}
}

void drawn_zero_page(memory_16 * memory)
{
	uint8_t	line_pos = ZERO_PAGE_LINE;
	uint8_t zp_col = ZERO_PAGE_COLUMN + 1;
	uint8_t zp_data = 0;

	tui_move_cursor_coord(line_pos, zp_col);
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=- ZERO PAGE =-=-=-=-=-=-=-=-=-=-=-");
	tui_move_cursor_coord(++line_pos, zp_col);
	tui_move_cursor_coord(++line_pos, zp_col);
	printf("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	tui_move_cursor_coord(++line_pos, zp_col);
	for (uint16_t i = 0x00; i < 0x100; i++) {
		if ((i % 0x10) == 0) {
			tui_move_cursor_coord(++line_pos, zp_col);
			printf("%02x  ", i);
		}

		zp_data = memory_read(memory, i);
		if (zp_data == previous_zero_page[i]) {
			printf("%02x ", zp_data);
		} else {
			tui_change_terminal_color(COLOR_BACK_BLUE);
			printf("%02x ", zp_data);
			tui_terminal_color_default();
		}
	}
}

void drawn_registers(memory_16 * memory, processor_6502 * processor)
{
	uint8_t line_pos = REGISTERS_LINE;
	uint8_t col_pos = REGISTERS_COLUMN + 1;

	char flag[9] = {0};
	
	u8_decimal_to_binary_string_(processor->reg_p, flag);	

	tui_move_cursor_coord(line_pos, col_pos);
	printf("=-=-=-=-=-=-=-=-=-=-=- 6502 REGISTERS  =-=-=-=-=-=-=-=-=-=");
	tui_move_cursor_coord(++line_pos, col_pos);
	printf("A:%02x\tX:%02x\tY:%02x\t\tFLAG:", processor->reg_a, processor->reg_x, processor->reg_y);
	show_formated_flags(processor->reg_p);
	tui_move_cursor_coord(++line_pos,col_pos);
	show_current_opcode(memory, processor->reg_pc);	
	tui_move_cursor_coord(++line_pos,col_pos);
	printf("Program Counter:\t\t0x%02x", processor->reg_pc);
	tui_move_cursor_coord(++line_pos,col_pos);
	printf("Cycles:\t\t\t%016lx", processor->cycles);

}

void down_right_panel()
{
	uint8_t line_pos = DOWN_RIGHT_PANEL_LINE;
	uint8_t col_pos = DOWN_RIGHT_PANEL_COLUMN + 1;
	tui_move_cursor_coord(line_pos, col_pos);
	tui_colorful_print("Press any key to continue. (\'q\' to exit): ",COLOR_BACK_BLUE);
}

void drawn_borders()
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

	// Drawn horizontal split
	tui_move_cursor_coord(HSPLIT_POSITION, 1);
	for (uint8_t i = 1; i < (SCREEN_WIDTH + 1); i++)
		printf("-");
}

void debugger_run(memory_16 * memory, processor_6502 * processor)
{
	drawn_borders();
	drawn_rom(memory, processor->reg_pc);
	drawn_zero_page(memory);
	drawn_registers(memory, processor);

	// Position where the cursor will rest for a little bit
	down_right_panel();
	update_previous_zero_page(memory);
}

void debugger_terminal_prepare()
{
	tui_terminal_init();
}

void debugger_terminal_finish()
{
	tui_terminal_cleanup();
}

