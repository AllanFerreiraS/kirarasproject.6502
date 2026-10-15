#include "../include/debugger.h"
#include "../include/terminal_ui.h"
#include <stdio.h>
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

void drawn_rom(memory_16 * memory, uint16_t reg_pc)
{
	uint8_t line_pos = ROM_LINE;
	uint8_t col_pos = ROM_COLUM + 1;

	tui_move_cursor_coord(line_pos, col_pos);
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=-= ROM -=-=-=-=-=-=-=-=-=-=-=-=-");
	tui_move_cursor_coord(++line_pos, col_pos);
	tui_move_cursor_coord(++line_pos, col_pos);
	printf("      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	tui_move_cursor_coord(++line_pos, col_pos);
	for (uint32_t i = 0x8000; i < 0x8100; i++) {
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
		printf("%02x ", memory->data[i]);
	}
}

void drawn_registers(processor_6502 * processor)
{
	uint8_t line_pos = REGISTERS_LINE;
	uint8_t col_pos = REGISTERS_COLUMN + 1;
	tui_move_cursor_coord(line_pos, col_pos);
	printf("=-=-=-=-=-=-=-=-=-=-=- 6502 REGISTERS  =-=-=-=-=-=-=-=-=-=");
	tui_move_cursor_coord(++line_pos, col_pos);
	printf("A:%02x\tX:%02x\tY:%02x\tFLAG:%02x", processor->reg_a, processor->reg_x, processor->reg_y, processor->reg_p);
	tui_move_cursor_coord(++line_pos,col_pos);
	printf("Program Counter:%02x", processor->reg_pc);
	tui_move_cursor_coord(++line_pos,col_pos);
	printf("OPCODE: LDA");
}

void down_right_panel()
{
	uint8_t line_pos = DOWN_RIGHT_PANEL_LINE;
	uint8_t col_pos = DOWN_RIGHT_PANEL_COLUMN + 1;
	tui_move_cursor_coord(line_pos, col_pos);
	tui_colorful_print("Press any key to continue. (\'q\' to exit): ", COLOR_BACK_CYAN);
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
	drawn_registers(processor);

	// Position where the cursor will rest for a little bit
	down_right_panel();
}

void debugger_terminal_prepare()
{
	tui_terminal_init();
}

void debugger_terminal_finish()
{
	tui_terminal_cleanup();
}

