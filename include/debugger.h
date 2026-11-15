#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdint.h>
#include "./disassemble.h"
#include "./terminal_ui.h"
#include "./hardwares/memory_16.h"
#include "./hardwares/processor_6502.h"

void debugger_print_formated_flags(const char *label, uint8_t flag);
void debugger_terminal_run(processor_6502 * processor, memory_16 * memory, disassemble_asm_list *asm_list, uint8_t stop);
void debugger_terminal_prepare();
void debugger_terminal_finish();

#endif
