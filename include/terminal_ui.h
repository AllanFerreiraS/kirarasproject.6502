#ifndef TERMINAL_UI_H_
#define TERMINAL_UI_H_

#define MOVE_UP			'A'
#define MOVE_DOWN		'B'
#define MOVE_RIGHT		'C'
#define MOVE_LEFT		'D'
#define MOVE_NEXT_LINE_BEGIN	'E'
#define MOVE_PREV_LINE_BEGIN	'F'
#define MOVE_COLUMN		'G'

#define COLOR_BACK_RED		"41"
#define COLOR_BACK_CYAN		"46"

#define ERASE_ENTIRE_SCREEN	"2J"

void tui_move_cursor(int step, char command);
void tui_move_cursor_home();
void tui_move_cursor_coord(int line, int column);
void tui_erase_screen(char * command);
void tui_colorful_print(char * text, char * color);
void tui_change_terminal_color(char * color);
void tui_terminal_color_default();
void tui_terminal_init();
void tui_terminal_cleanup();

#endif
