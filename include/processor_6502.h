#ifndef PROCESSOR_6502_H_
#define PROCESSOR_6502_H_

#include <stdint.h>
#include "memory_16.h"

typedef uint8_t 	bits_8;
typedef uint16_t	bits_16;
typedef uint64_t	bits_64;

// reg_p bits: 
//	7: Negative
//	6: Overflow
//	5: (No CPU Effect | Always 1)
//	4: (No CPU Effect | Break Flag)
//	3: Decimal
//	2: Interrupt Disable
//	1: Zero
//	0: Carry
typedef struct 	{	
	bits_8	reg_a;	
	bits_8 	reg_x;	
	bits_8 	reg_y;
	bits_8	reg_sp;		//Stack Pointer
	bits_8	reg_p;		//Status Register
	bits_16	reg_pc;		//Program Counter
	bits_64 cycles;
} processor_6502;

// Public Functions
void init_6502(processor_6502 * processor);
void execute_opcode(processor_6502 * processor, memory_16 * memory);

#endif
