#include <stdio.h>
#include "./include/processor_6502.h"
#include "./include/memory_16.h"
#include "./include/tui.h"

int main()
{
	
	char 		stop_flag;
	processor_6502 	cpu;
	memory_16	memory;

	write_rom(&memory, "./tests/rom.bin");
	print_memory(&memory);
	init_6502(&cpu);
	
	terminal_init();

	while(1) {
		execute_opcode(&cpu, &memory);
		
		//drawn_zero_page(&memory);
		monitor_emulator(&memory, &cpu);
		
		//printf(":");
		scanf("%c", &stop_flag);
		if (stop_flag == 'q')
			break;
	}
	
	terminal_cleanup();

	printf("\n========== RESULTS ==========\n");
	//print_zero_page(&memory);
	print_memory(&memory);
	printf("\n");
	printf("PC:%x\n", cpu.reg_pc);
	printf("A:%x\n", cpu.reg_a);

	return 0;
}




