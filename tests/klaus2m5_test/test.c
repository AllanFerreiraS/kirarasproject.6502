#include "./../../emulators/raw/raw.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TRUE	1
#define FALSE	0

#define PC_ADDR		0x0400
#define MEM_ADDR	0x0000
#define KLAUS_BIN	"6502_interrupt_test"

int main ()
{
	
	processor_6502 	cpu;
	memory_16	mem;

	infra_raw	emu = {
		.cpu = cpu,
		.memory = mem,
	};

	if (! write_rom(&(emu.memory), KLAUS_BIN, MEM_ADDR) ) {
		printf("Error: \'%s\' not found.\n", KLAUS_BIN);
		exit(EXIT_FAILURE);
	}

	emu.cpu.bus = (buses) { bus_raw_read, bus_raw_write, &emu  };
	processor_6502_manual_initialization(&(emu.cpu), PC_ADDR);
	
	uint16_t previous_opcode = 0;
	uint64_t counter = 0;
	char	 stop_flag;

	debugger_terminal_prepare();
	while (1) {
		previous_opcode = emu.cpu.reg_pc;
		processor_6502_execute_opcode(&(emu.cpu));
		debugger_run(&(emu.memory), &(emu.cpu));
		fflush(stdout);
		counter++;
		
		scanf("%c", &stop_flag);
		if (stop_flag == 'q')
			break;

		if (previous_opcode == emu.cpu.reg_pc)
			break;
	}
	debugger_terminal_finish();

	printf("Klaus2m5 6502 Functional Test\n");
	printf("-----------------------------\n");
	printf("Instructions executed:\t %ld\n", counter);
	printf("Cycles elapsed:       \t 0x%016lx\n", emu.cpu.cycles);
	printf("Final PC:             \t 0x%04x\n", emu.cpu.reg_pc);
	printf("RESULT ----------------------\n\n");
	if (emu.cpu.reg_pc == 0x3469) {
		printf("\t\t; S U C C E S S ************************************************\n");
                printf("\t\t; -------------\n");
                printf("\t\t\t success         ;if you get here everything went well\n");
		printf("3469 : 4c6934\t >       jmp *           ;test passed, no errors\n");
	}
	else {
		printf(">> FAILED\n");
	}

	return 0;
}
