#ifndef DEBUGGER_H_
#define DEBUGGER_H_
#include "./../disassemble/disassemble.h"
#include "./../terminal_ui/terminal_ui.h"
#include "./../../include/utils.h"
#include "./../../include/hardwares/memory.h"
#include "./../../include/hardwares/cpu6502.h"

void debugger_print_formated_flags(const char *label, u8 flag);
void debugger_terminal_run(struct cpu6502 *cpu, struct memory * memory, struct disasm_list *asm_list, u8 stop);
void debugger_terminal_prepare();
void debugger_terminal_finish();

#endif
