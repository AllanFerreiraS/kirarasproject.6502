#ifndef DEBUGGER_H_
#define DEBUGGER_H_

#include <stdint.h>
#include "./memory_16.h"
#include "./processor_6502.h"

void show_current_opcode(memory_16 * memory, uint16_t reg_pc);
void update_previous_zero_page(memory_16 * memory);
void drawn_rom(memory_16 * memory, uint16_t reg_pc);
void drawn_zero_page(memory_16 * memory);
void drawn_registers(memory_16 * memory, processor_6502 * processor);
void debugger_run(memory_16 * memory, processor_6502 * processor);
void debugger_terminal_prepare();
void debugger_terminal_finish();

#endif
