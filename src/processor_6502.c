#include "../include/processor_6502.h"
#include "../include/memory_16.h"
#include <stdio.h>

#define NUM_OPCODES	0x100
#define FLAG_NEG	0x80
#define FLAG_OVF	0x40
#define FLAG_BRK	0x10
#define FLAG_DEC	0x08
#define FLAG_INT	0x04
#define FLAG_ZER	0x02
#define FLAG_CAR	0x01

/*
 * Description: Function Pointers to handle Addressing Modes and Opcodes.
 */
typedef bits_16	(*addr_fn) (processor_6502 * processor, memory_16 * memory);
typedef void	(*op_fn) (processor_6502 * processor, memory_16 * memory, bits_16 addr);

/*
 * Description: Opcode Structure.
 */
typedef struct {
	const char	*op_name;
	op_fn		op;
	addr_fn		mode;
	bits_8		cycle;
} opcode_entry;

// Addressing Mode Functions
static bits_16 mode_immediate(processor_6502 * processor, memory_16 * memory);
static bits_16 mode_zero_page(processor_6502 * processor, memory_16 * memory);
static bits_16 mode_zero_page_x(processor_6502 * processor, memory_16 * memory);
static bits_16 mode_absolute(processor_6502 * processor, memory_16 * memory);

// OPCODE Functions
static void op_lda(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_sta(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_ldx(processor_6502 * processor, memory_16 * memory, bits_16 addr);

// Flag
static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition);
static void set_nz(processor_6502 * processor, bits_8 data);

// Others
static uint16_t little_endian_solver(uint16_t low, uint16_t high);

/*
 * Description: A table containing all OPCODES
 */
static const opcode_entry op_table[NUM_OPCODES] = {
	[0xA9]	=	{ "LDA", op_lda, mode_immediate, 	2 }, // PASSED
	[0xA5]	=	{ "LDA", op_lda, mode_zero_page, 	3 }, // PASSED
	[0xB5]	=	{ "LDA", op_lda, mode_zero_page_x, 	4 }, // PASSED
	[0xAD]	=	{ "LDA", op_lda, mode_absolute,		4 }, // PASSED
	[0x85]	=	{ "STA", op_sta, mode_zero_page,	3 }, // PASSED 
	[0x95] 	=	{ "STA", op_sta, mode_zero_page_x,	4 }, // PASSED 
	[0xA2]	=	{ "LDX", op_ldx, mode_immediate, 	2 }, // PASSED
	[0xA6]	=	{ "LDX", op_ldx, mode_zero_page, 	3 }  // PASSED
};

static uint16_t little_endian_solver(uint16_t low, uint16_t high)
{
	return ( (high << 8) | low );
}


// >> FLAGS <<

/*
 * Description: Set flag using the MASK and the condition as parameter
 */
static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition)
{
	if (condition)
		processor->reg_p |= flag_mask;
	else
		processor->reg_p &= (~flag_mask);
}

/*
 * Description: Set ZERO FLAG and NEGATIVE FLAG
 */
static void set_nz(processor_6502 * processor, bits_8 data)
{
	set_flag(processor, FLAG_ZER, (data == 0));
	set_flag(processor, FLAG_NEG, ((data & FLAG_NEG) > 0));
}

// >> ADDRESSING MODES <<

/*
 * Description: Immediate Mode Addressing.
 */
static bits_16 mode_immediate(processor_6502 * processor, memory_16 * memory) 
{
	bits_16 addr = processor->reg_pc;
	processor->reg_pc++;
	return addr;
}

/*
 * Description: Zero Page Mode Addressing.
 */
static bits_16 mode_zero_page(processor_6502 * processor, memory_16 * memory)
{
	bits_16 addr = memory_read(memory, processor->reg_pc);
	processor->reg_pc++;
	return addr;
}

/*
 * Description: Zero Page Mode + Register X.
 */
static bits_16 mode_zero_page_x(processor_6502 * processor, memory_16 * memory)
{
	bits_16 addr = memory_read(memory, processor->reg_pc);
	addr = (addr + processor->reg_x) & 0xFF;
	processor->reg_pc++;
	return addr;
}

/*
 * Description: Absolute. Low Addr first, High addr later.
 */
static bits_16 mode_absolute(processor_6502 * processor, memory_16 * memory)
{
	bits_16 low = memory_read(memory, processor->reg_pc);
	processor->reg_pc++;
	bits_16 high = memory_read(memory, processor->reg_pc);
	bits_16 addr = little_endian_solver(low, high);	
	processor->reg_pc++;
	return addr;
}

// >> OPCODES <<

/*
 * Description: Loads a value to register A.
 */
static void op_lda(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_a = memory_read(memory, addr);
	set_nz(processor, processor->reg_a);
}

/*
 * Description: Store the value of register A in a memory address.
 */
static void op_sta(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	memory_write(memory, addr, processor->reg_a);
}

/*
 * Description: Loads a value to register X.
 */
static void op_ldx(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_x = memory_read(memory, addr);
	set_nz(processor, processor->reg_x);
}

// >> PUBLIC FUNCTIONS <<

void init_6502(processor_6502 * processor)
{	
	processor->reg_a  =	0X00;
	processor->reg_x  = 	0X00;
	processor->reg_y  = 	0X00;
	processor->reg_sp = 	0XFD;
	processor->reg_p  =	0X24;
	processor->reg_pc = 	0X8000;
	processor->cycles = 	0X00;	
	//Escrevendo na memoria
}

void execute_opcode(processor_6502 * processor, memory_16 * memory)
{	
	bits_8 opcode = memory_read(memory, processor->reg_pc);
	processor->reg_pc++;

	opcode_entry execute = op_table[opcode];
	if (execute.op == NULL) {
		//printf("UNDEFINED OPCODE...\n");
		return;
	}
	//printf("Executing %s\n", execute.op_name);
	bits_16 address = execute.mode(processor, memory);
	execute.op(processor, memory, address);
	processor->cycles += execute.cycle;

	//printf(">> A:%02x  X:%02x  Y:%02x  P:%02x\n", processor->reg_a, processor->reg_x, processor->reg_y, processor->reg_p);
}

