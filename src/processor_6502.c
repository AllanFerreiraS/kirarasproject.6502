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
typedef bits_16	(*addr_fn) (processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
typedef void	(*op_fn) (processor_6502 * processor, memory_16 * memory, bits_16 addr);

/*
 * Description: Opcode Structure.
 */
typedef struct {
	const char	*op_name;
	op_fn		op;
	addr_fn		mode;
	bits_8		cycle;
	bits_8		page_crossed_cycle;
} opcode_entry;

// Addressing Mode Functions
static bits_16 mode_immediate(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_zero_page(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_zero_page_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_zero_page_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_absolute(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_absolute_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_absolute_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_indirect_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_indirect_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);
static bits_16 mode_implied(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed);

// OPCODE Functions
// Access:
static void op_lda(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_ldx(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_ldy(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_sta(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_stx(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_sty(processor_6502 * processor, memory_16 * memory, bits_16 addr);
// Transfer:
static void op_tax(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_tay(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_tsx(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_txa(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_txs(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_tya(processor_6502 * processor, memory_16 * memory, bits_16 addr);
// Arithmetic:
static void op_adc(processor_6502 * processor, memory_16 * memory, bits_16 addr);
static void op_sbc(processor_6502 * processor, memory_16 * memory, bits_16 addr);

// Flag
static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition);
static void set_nz(processor_6502 * processor, bits_8 data);

// Others
static uint16_t little_endian_solver(uint16_t low, uint16_t high);
static uint8_t 	is_page_crossed(uint16_t before, uint16_t after);

// Handle (Helpers)
static void handle_op_adc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_adc_binary_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_binary_mode(processor_6502 * p, uint8_t data);


/*
 * Description: A table containing all OPCODES
 */
static const opcode_entry op_table[NUM_OPCODES] = {
	[0xA9]	=	{ "LDA", op_lda, mode_immediate, 	2,	0 }, // PASSED 
	[0xA5]	=	{ "LDA", op_lda, mode_zero_page, 	3,	0 }, // PASSED
	[0xB5]	=	{ "LDA", op_lda, mode_zero_page_x, 	4,	0 }, //	PASSED
	[0xAD]	=	{ "LDA", op_lda, mode_absolute,		4,	0 }, // PASSED
	[0xBD]	=	{ "LDA", op_lda, mode_absolute_x,	4,	1 }, // PASSED
	[0xB9]	=	{ "LDA", op_lda, mode_absolute_y,	4,	1 }, // PASSED
	[0xA1]	=	{ "LDA", op_lda, mode_indirect_x,	6,	0 }, // PASSED
	[0xB1]	=	{ "LDA", op_lda, mode_indirect_y, 	5,	1 }, // PASSED
	[0xA2]	=	{ "LDX", op_ldx, mode_immediate, 	2,	0 }, // PASSED
	[0xA6]	=	{ "LDX", op_ldx, mode_zero_page, 	3,	0 }, // PASSED
	[0xB6]	=	{ "LDX", op_ldx, mode_zero_page_y,	4,	0 }, // PASSED
	[0xAE]	=	{ "LDX", op_ldx, mode_absolute,		4,	0 }, // PASSED
	[0xBE]	=	{ "LDX", op_ldx, mode_absolute_y,	4,	1 }, // PASSED
	[0xA0]	=	{ "LDY", op_ldy, mode_immediate,	2,	0 }, // PASSED 
	[0xA4]	=	{ "LDY", op_ldy, mode_zero_page,	3,	0 }, // PASSED
	[0xB4]	=	{ "LDY", op_ldy, mode_zero_page_x, 	4,	0 }, // PASSED
	[0xAC]	=	{ "LDY", op_ldy, mode_absolute,		4,	0 }, // PASSED
	[0xBC]	=	{ "LDY", op_ldy, mode_absolute_x,	4,	1 }, // PASSED
	[0x85]	=	{ "STA", op_sta, mode_zero_page,	3,	0 }, // PASSED 
	[0x95]	=	{ "STA", op_sta, mode_zero_page_x,	4,	0 }, // PASSED
	[0x8D]	=	{ "STA", op_sta, mode_absolute,		4,	0 }, // PASSED
	[0x9D]	=	{ "STA", op_sta, mode_absolute_x,	5,	0 }, // PASSED
	[0x99]	=	{ "STA", op_sta, mode_absolute_y,	5,	0 }, // PASSED
	[0x81]	=	{ "STA", op_sta, mode_indirect_x,	6,	0 }, // PASSED
	[0x91]	=	{ "STA", op_sta, mode_indirect_y,	6,	0 }, // PASSED
	[0x86]	=	{ "STX", op_stx, mode_zero_page,	3,	0 }, // PASSED
	[0x96]	=	{ "STX", op_stx, mode_zero_page_y,	4,	0 }, // PASSED
	[0x8E]	=	{ "STX", op_stx, mode_absolute,		4,	0 }, // PASSED
	[0x84]	=	{ "STY", op_sty, mode_zero_page,	3,	0 }, // PASSED
	[0x94]	=	{ "STY", op_sty, mode_zero_page_x,	4,	0 }, // PASSED
	[0x8C]	=	{ "STY", op_sty, mode_absolute,		4,	0 }, // PASSED
	[0xAA]	=	{ "TAX", op_tax, mode_implied,		2,	0 }, // PASSED
	[0xA8]	=	{ "TAY", op_tay, mode_implied,		2,	0 }, // PASSED
	[0xBA]	=	{ "TSX", op_tsx, mode_implied, 		2,	0 }, // PASSED
	[0x8A]	=	{ "TXA", op_txa, mode_implied, 		2,	0 }, // PASSED
	[0x9A]	=	{ "TXS", op_txs, mode_implied,		2,	0 }, // PASSED
	[0x98]	=	{ "TYA", op_tya, mode_implied,		2,	0 }, // PASSED
	[0x69]	=	{ "ADC", op_adc, mode_immediate,	2,	0 }, // PASSED
	[0x65]	=	{ "ADC", op_adc, mode_zero_page,	3,	0 }, // PASSED
	[0x75]	=	{ "ADC", op_adc, mode_zero_page_x,	4,	0 }, // PASSED
	[0x6D]	=	{ "ADC", op_adc, mode_absolute,		4,	0 }, // PASSED
	[0x7D]	=	{ "ADC", op_adc, mode_absolute_x,	4,	1 }, // PASSED
	[0x79]	=	{ "ADC", op_adc, mode_absolute_y,	4,	1 }, // PASSED
	[0x61]	=	{ "ADC", op_adc, mode_indirect_x,	6,	0 }, // PASSED
	[0x71]	=	{ "ADC", op_adc, mode_indirect_y,	5,	1 }, // PASSED
	[0xE9]	=	{ "SBC", op_sbc, mode_immediate,	2,	0 }, // PASSED	 
	[0xE5]	=	{ "SBC", op_sbc, mode_zero_page,	3,	0 }, // PASSED 
	[0xF5]	=	{ "SBC", op_sbc, mode_zero_page_x,	4,	0 }, // PASSED
	[0xED]	=	{ "SBC", op_sbc, mode_absolute,		4,	0 }, // PASSED
	[0xFD]	=	{ "SBC", op_sbc, mode_absolute_x,	4,	1 }, // PASSED
	[0xF9]	=	{ "SBC", op_sbc, mode_absolute_y, 	4,	1 }, // PASSED
	[0xE1]	=	{ "SBC", op_sbc, mode_indirect_x,	6,	0 }, // PASSED
	[0xF1]	=	{ "SBC", op_sbc, mode_indirect_y,	5,	1 }, // PASSED

};

static uint16_t little_endian_solver(uint16_t low, uint16_t high) 
{
	return ( (high << 8) | low );
}

static uint8_t is_page_crossed(uint16_t before, uint16_t after)
{
	return ( (before & 0xFF00) != (after & 0xFF00) );
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

static bits_16 mode_immediate(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = processor->reg_pc;
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_zero_page(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = memory_read(memory, processor->reg_pc);
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_zero_page_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = memory_read(memory, processor->reg_pc);
	addr = (addr + processor->reg_x) & 0xFF;
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_zero_page_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = memory_read(memory, processor->reg_pc);
	addr = (addr + processor->reg_y) & 0xFF;
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_absolute(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 low = memory_read(memory, processor->reg_pc);
	processor->reg_pc++;
	bits_16 high = memory_read(memory, processor->reg_pc);
	bits_16 addr = little_endian_solver(low, high);	
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_absolute_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = mode_absolute(processor, memory, if_page_crossed);
	if ( is_page_crossed(addr, (addr + processor->reg_x) & 0xFFFF) )
		*if_page_crossed = (~0);
	addr = (addr + processor->reg_x) & 0xFFFF;
	return addr;
}

static bits_16 mode_absolute_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = mode_absolute(processor, memory, if_page_crossed);
	if ( is_page_crossed(addr, (addr + processor->reg_y) & 0xFFFF) )
		*if_page_crossed = (~0);
	addr = (addr + processor->reg_y) & 0xFFFF;
	return addr;
}

static bits_16 mode_indirect_x(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 low_addr = memory_read(memory, processor->reg_pc);
	low_addr = (low_addr + processor->reg_x) & 0xFF;
	bits_16 low = memory_read(memory, low_addr);
	bits_16 high = memory_read(memory, (low_addr + 1) & 0xFF);
	bits_16 addr = little_endian_solver(low, high);
	*if_page_crossed = 0;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_indirect_y(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 low_addr = memory_read(memory, processor->reg_pc);
	bits_16 low = memory_read(memory, low_addr);
	bits_16 high = memory_read(memory, (low_addr + 1) & 0xFF);
	bits_16 addr = little_endian_solver(low, high);
	if ( is_page_crossed(addr, (addr + processor->reg_y) & 0xFFFF) )
		*if_page_crossed = (~0);
	addr = (addr + processor->reg_y) & 0xFFFF;
	processor->reg_pc++;
	return addr;
}

static bits_16 mode_implied(processor_6502 * processor, memory_16 * memory, bits_8 *if_page_crossed)
{
	bits_16 addr = processor->reg_pc;
	*if_page_crossed = 0;
	return addr;
}

// >> HANDLE FUNCTIONS <<

static void handle_op_adc_decimal_mode(processor_6502 * p, uint8_t data)
{
	// For more info: https://6502.org/tutorials/decimal_mode.html#A
	uint16_t r 	= (p->reg_a & 0x0F) + (data & 0X0F) + (p->reg_p & FLAG_CAR);
	uint16_t r_bin	= (p->reg_a) + (data) + (p->reg_p & FLAG_CAR);
	uint16_t old_a 	= p->reg_a;

	set_flag(p, FLAG_ZER, ((r_bin & 0xFF) == 0x00));
				
	if (r >= 0x0A)
		r = ((r + 0x06) & 0x0F) + 0x10;	

	r = (p->reg_a & 0xF0) + (data & 0xF0) + r;

	set_flag(p, FLAG_NEG, (r & FLAG_NEG));
	set_flag(p, FLAG_OVF, (r ^ old_a) & (r ^ ((uint16_t)data)) & 0x0080);	

	if (r >= 0xA0)
		r = r + 0x60;
	
	set_flag(p, FLAG_CAR, (r >= 0x100));

	p->reg_a = r & 0xFF;
}

static void handle_op_sbc_decimal_mode(processor_6502 * p, uint8_t data)
{
	// For more info: https://6502.org/tutorials/decimal_mode.html#A
	uint16_t r 	= (p->reg_a & 0x0F) - (data & 0X0F) + (p->reg_p & FLAG_CAR) - 1;
	uint16_t r_bin	= (p->reg_a) - (data) + (p->reg_p & FLAG_CAR) - 1;
	uint16_t old_a 	= p->reg_a;

	set_flag(p, FLAG_NEG, (r_bin & FLAG_NEG));
	set_flag(p, FLAG_CAR, (~((int16_t)r_bin)) < 0x00);
	set_flag(p, FLAG_ZER, ((r_bin & 0xFF) == 0x00));
	set_flag(p, FLAG_OVF, (r_bin ^ old_a) & (r_bin ^ (~(uint16_t)data)) & 0x0080);
			
	if ((int16_t)r < 0)
		r = ((r - 0x06) & 0x0F) - 0x10;	

	r = (p->reg_a & 0xF0) - (data & 0xF0) + r;
	if ((int16_t)r < 0)
		r = (r - 0x60) & 0xFF;
	

	p->reg_a = r & 0xFF;
}

static void handle_op_adc_binary_mode(processor_6502 * p, uint8_t data)
{
	uint16_t r 	= ((uint16_t)p->reg_a) + ((uint16_t)data) + ((uint16_t)(p->reg_p & FLAG_CAR));
	uint16_t old_a 	= p->reg_a;
	p->reg_a 	= r & 0xFF;

	set_flag(p, FLAG_CAR, (r > 0xFF));
	set_flag(p, FLAG_ZER, (p->reg_a & 0xFF) == 0);
	set_flag(p, FLAG_OVF, (r ^ old_a) & (r ^ (uint16_t)data) & 0x0080);
	set_flag(p, FLAG_NEG, (p->reg_a & FLAG_NEG) > 0);
}

static void handle_op_sbc_binary_mode(processor_6502 * p, uint8_t data)
{
	uint16_t r 	= ((uint16_t)p->reg_a) + ~((uint16_t)data) + ((uint16_t)(p->reg_p & FLAG_CAR));
	uint16_t old_a 	= p->reg_a;
	p->reg_a	= r & 0xFF;

	set_flag(p, FLAG_CAR, (~((int16_t)r)) < 0x00);
	set_flag(p, FLAG_ZER, (p->reg_a & 0xFF) == 0);
	set_flag(p, FLAG_OVF, (r ^ old_a) & (r ^ (~(uint16_t)data)) & 0x0080);
	set_flag(p, FLAG_NEG, (p->reg_a & FLAG_NEG) > 0);
}

// >> OPCODES <<

static void op_lda(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_a = memory_read(memory, addr);
	set_nz(processor, processor->reg_a);
}

static void op_ldx(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_x = memory_read(memory, addr);
	set_nz(processor, processor->reg_x);
}

static void op_ldy(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_y = memory_read(memory, addr);
	set_nz(processor, processor->reg_y);
}

static void op_sta(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	memory_write(memory, addr, processor->reg_a);
}

static void op_stx(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	memory_write(memory, addr, processor->reg_x);
}

static void op_sty(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	memory_write(memory, addr, processor->reg_y);
}

static void op_tax(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_x = processor->reg_a;
	set_nz(processor, processor->reg_x);
}

static void op_tay(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_y = processor->reg_a;
	set_nz(processor, processor->reg_y);
}

static void op_tsx(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_x = processor->reg_sp;
	set_nz(processor, processor->reg_x);
}

static void op_txa(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_a = processor->reg_x;
	set_nz(processor, processor->reg_a);
}

static void op_txs(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_sp = processor->reg_x;
}

static void op_tya(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	processor->reg_a = processor->reg_y;
	set_nz(processor, processor->reg_a);
}


static void op_adc(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	uint8_t data = memory_read(memory, addr);
	if (processor->reg_p & FLAG_DEC)
		handle_op_adc_decimal_mode(processor, data);
	else
		handle_op_adc_binary_mode(processor, data);
}

static void op_sbc(processor_6502 * processor, memory_16 * memory, bits_16 addr)
{
	uint8_t data = memory_read(memory, addr);
	if (processor->reg_p & FLAG_DEC)
		handle_op_sbc_decimal_mode(processor, data);
	else
		handle_op_sbc_binary_mode(processor, data);
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
	bits_8 if_page_crossed = 0;
	bits_8 opcode = memory_read(memory, processor->reg_pc);
	processor->reg_pc++;

	opcode_entry execute = op_table[opcode];
	if (execute.op == NULL) {
		//printf("UNDEFINED OPCODE...\n");
		return;
	}
	//printf("Executing %s\n", execute.op_name);
	bits_16 address = execute.mode(processor, memory, &if_page_crossed);
	execute.op(processor, memory, address);
	// Não acumular ciclos?
	processor->cycles = execute.cycle + (if_page_crossed & execute.page_crossed_cycle);

	//printf(">> A:%02x  X:%02x  Y:%02x  P:%02x\n", processor->reg_a, processor->reg_x, processor->reg_y, processor->reg_p);
}

