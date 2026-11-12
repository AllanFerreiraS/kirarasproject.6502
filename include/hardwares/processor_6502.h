#ifndef PROCESSOR_6502_H_
#define PROCESSOR_6502_H_

//#include "memory_16.h"
#include "./../buses.h"
#include <stdint.h>

// Interrupt Pins
#define PROCESSOR_6502_POWER_UP				1
#define PROCESSOR_6502_RESET				0

// Addresses Mode Arbitrary Codes
#define PROCESSOR_6502_ADDR_MODE_IMMEDIATE_CODE		0
#define PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_CODE		1
#define PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_X_CODE	2
#define PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_Y_CODE	3
#define PROCESSOR_6502_ADDR_MODE_ABSOLUTE_CODE		4
#define PROCESSOR_6502_ADDR_MODE_ABSOLUTE_X_CODE	5
#define	PROCESSOR_6502_ADDR_MODE_ABSOLUTE_Y_CODE	6
#define PROCESSOR_6502_ADDR_MODE_INDIRECT_CODE		7
#define PROCESSOR_6502_ADDR_MODE_INDIRECT_X_CODE	8
#define PROCESSOR_6502_ADDR_MODE_INDIRECT_Y_CODE	9
#define PROCESSOR_6502_ADDR_MODE_IMPLIED_CODE		10
#define PROCESSOR_6502_ADDR_MODE_RELATIVE_CODE		11
#define PROCESSOR_6502_ADDR_MODE_ERROR_CODE		12

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

	bits_8 	irq_pin;
	bits_8	nmi_pin;
	bits_8	nmi_previous;
	bits_8	nmi_request;	// TRUE or FALSE
	bits_64 cycles;
	buses	bus;	
} processor_6502;


// Public Functions
void 	processor_6502_initialization(processor_6502 * processor, uint8_t is_power_up);
void 	processor_6502_manual_initialization(processor_6502 * processor, uint16_t start_pc_addr);
void 	processor_6502_execute_opcode(processor_6502 * processor);
uint8_t	processor_6502_get_opcode_infos(uint8_t opcode, char **name, char **mode, uint8_t *mode_code);

#endif
