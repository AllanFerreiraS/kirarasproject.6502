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
	print_memory(&memory);
	init_6502(&cpu);
	
	debugger_terminal_prepare();

	while(1) {

		//drawn_zero_page(&memory);
		debugger_run(&memory, &cpu);

		execute_opcode(&cpu, &memory);		
		scanf("%c", &stop_flag);
		if (stop_flag == 'q')
			break;
	}
	
	debugger_terminal_finish();

	printf("\n========== RESULTS ==========\n");
	//print_zero_page(&memory);
	print_memory(&memory);
	printf("\n");
	printf("PC:%x\n", cpu.reg_pc);
	printf("A:%x\n", cpu.reg_a);

	return 0;
}




