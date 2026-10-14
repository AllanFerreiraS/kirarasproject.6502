#ifndef TUI_H_
#define TUI_H_

#include "../include/memory_16.h"
#include "../include/processor_6502.h"

#define MOVE_UP			'A'
#define MOVE_DOWN		'B'
#define MOVE_RIGHT		'C'
#define MOVE_LEFT		'D'
#define MOVE_NEXT_LINE_BEGIN	'E'
#define MOVE_PREV_LINE_BEGIN	'F'
#define MOVE_COLUMN		'G'

#define ERASE_ENTIRE_SCREEN	"2J"

void move_cursor(int step, char command);
void move_cursor_home();
void move_cursor_coord(int line, int column);
void erase_screen(char * command);
void terminal_init();
void terminal_cleanup();
void drawn_zero_page(memory_16 * memory);
void drawn_registers(processor_6502 * processor);
void monitor_emulator(memory_16 * memory, processor_6502 * processor);
#endif
