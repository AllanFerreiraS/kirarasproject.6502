#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include "./../disassemble/disassemble.h"
#include "./../terminal_ui/terminal_ui.h"
#include "./../../include/hardwares/memory_16.h"
#include "./../../include/hardwares/cpu6502.h"
#include <stdint.h>

void debugger_print_formated_flags(const char *label, uint8_t flag);
void debugger_terminal_run(struct cpu6502 *cpu, memory_16 * memory, disassemble_asm_list *asm_list, uint8_t stop);
void debugger_terminal_prepare();
void debugger_terminal_finish();

#endif
