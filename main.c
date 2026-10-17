#include <stdio.h>
#include "./include/processor_6502.h"
#include "./include/memory_16.h"
#include "./include/debugger.h"

int main()
{	
	char 		stop_flag;
	processor_6502 	cpu;
	memory_16	memory;

	write_rom(&memory, "./tests/rom.bin");
	init_6502(&cpu);
	
	memory.data[0x80f0] = 0xFF;

	debugger_terminal_prepare();
	while(1) {
		debugger_run(&memory, &cpu);
		execute_opcode(&cpu, &memory);		
		scanf("%c", &stop_flag);
		if (stop_flag == 'q')
			break;
	}	
	debugger_terminal_finish();
	return 0;
}




