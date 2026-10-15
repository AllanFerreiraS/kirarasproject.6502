#include "../include/terminal_ui.h"
#include <stdio.h>
#include <stdint.h>

#define ESC			'\x1B'
#define MOVE_POS_HOME		'H'

void tui_move_cursor(int step, char command)
{
	printf("%c[%d%c", ESC, step, command);
}

void tui_move_cursor_home()
{
	printf("%c[%c", ESC, MOVE_POS_HOME);
}

void tui_move_cursor_coord(int line, int column)
{
	printf("%c[%d;%dH", ESC, line, column);
}

void tui_erase_screen(char * command)
{
	printf("%c[%s", ESC, command);
}

void tui_colorful_print(char * text, char * color)
{
	printf("%c[%sm%s%c[0m", ESC, color, text, ESC);
}

void tui_change_terminal_color(char * color)
{
	printf("%c[%sm", ESC, color);
}

void tui_terminal_color_default()
{
	printf("%c[0m", ESC);
}

void tui_terminal_init()
{
	printf("\033[?1049h");
    	tui_erase_screen(ERASE_ENTIRE_SCREEN);
    	tui_move_cursor_home();
    	fflush(stdout);
}

void tui_terminal_cleanup()
{
    	printf("\033[?1049l");
   	fflush(stdout);
}

