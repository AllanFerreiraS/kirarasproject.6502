#ifndef PROCESSOR_6502_H_
#define PROCESSOR_6502_H_

#include "memory_16.h"
#include "./../buses.h"
#include <stdint.h>

typedef uint8_t 	bits_8;
typedef uint16_t	bits_16;
typedef uint64_t	bits_64;

// reg_p bits: 
//	7: Negative
//	6: Overflow
//	5: (No CPU Effect | Always 1)
//	4: (No CPU Effect | Break Flag)
//	3: Decimal			Thankfully NES does not use this SH%T.
//	2: Interrupt Disable
//	1: Zero
//	0: Carry

// Decimal. Affects the D Flag: CLD, PLP, RTI, and SED. (BRK)
// Decimal. Affected by D flag: ADC and SBC

typedef struct 	{	
	bits_8	reg_a;	
	bits_8 	reg_x;	
	bits_8 	reg_y;
	bits_8	reg_sp;		//Stack Pointer
	bits_8	reg_p;		//Status Register
	bits_8  read_write;	//0 for read, 1 for write
	bits_16	reg_pc;		//Program Counter
	bits_64 cycles;
	buses	bus;	
} processor_6502;


// Public Functions
void init_6502(processor_6502 * processor);
void execute_opcode(processor_6502 * processor, memory_16 * memory);

#endif
