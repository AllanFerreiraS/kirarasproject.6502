#include "./../../include/hardwares/processor_6502.h"
#include "./../../include/buses.h"
#include "./../../include/hardwares/memory_16.h"
#include <stdio.h>

#define UNUSED(x)	(void)(x)

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
typedef bits_16	(*addr_fn) (processor_6502 * processor, bits_8 force_oops);
typedef void	(*op_fn) (processor_6502 * processor, bits_16 addr);

/*
 * Description: Opcode Structure.
 */
typedef struct {
	const char	*op_name;
	op_fn		op;
	addr_fn		mode;
	bits_8		force_oops; // Force Dummy Cycles;
} opcode_entry;

// Bus
static bits_8	processor_read(processor_6502 * processor, uint16_t address);
static void	processor_write(processor_6502 * processor, uint16_t address, uint8_t data);

// Addressing Mode Functions
static bits_16 mode_immediate(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_zero_page(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_zero_page_x(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_zero_page_y(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_absolute(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_absolute_x(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_absolute_y(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_indirect_x(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_indirect_y(processor_6502 * processor, bits_8 force_oops);
static bits_16 mode_implied(processor_6502 * processor, bits_8 force_oops);

// OPCODE Functions
// Access:
static void op_lda(processor_6502 * processor, bits_16 addr);
static void op_ldx(processor_6502 * processor, bits_16 addr);
static void op_ldy(processor_6502 * processor, bits_16 addr);
static void op_sta(processor_6502 * processor, bits_16 addr);
static void op_stx(processor_6502 * processor, bits_16 addr);
static void op_sty(processor_6502 * processor, bits_16 addr);
// Transfer:
static void op_tax(processor_6502 * processor, bits_16 addr);
static void op_tay(processor_6502 * processor, bits_16 addr);
static void op_tsx(processor_6502 * processor, bits_16 addr);
static void op_txa(processor_6502 * processor, bits_16 addr);
static void op_txs(processor_6502 * processor, bits_16 addr);
static void op_tya(processor_6502 * processor, bits_16 addr);
// Arithmetic:
static void op_adc(processor_6502 * processor, bits_16 addr);
static void op_sbc(processor_6502 * processor, bits_16 addr);
static void op_inc(processor_6502 * processor, bits_16 addr);
static void op_dec(processor_6502 * processor, bits_16 addr);

// Flag
static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition);
static void set_nz(processor_6502 * processor, bits_8 data);

// Others
static uint16_t little_endian_solver(uint16_t low, uint16_t high, uint8_t add, uint16_t *carry);
static uint8_t oops(uint8_t force_oops, uint8_t condition);


// Handle (Helpers)
static void handle_op_adc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_adc_binary_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_binary_mode(processor_6502 * p, uint8_t data);


/*
 * Description: A table containing all OPCODES
 */
static const opcode_entry op_table[NUM_OPCODES] = {
	[0xA9]	=	{ "LDA", op_lda, mode_immediate, 	0 }, // PASSED
	[0xA5]	=	{ "LDA", op_lda, mode_zero_page, 	0 }, // PASSED
	[0xB5]	=	{ "LDA", op_lda, mode_zero_page_x, 	0 }, //	PASSED
	[0xAD]	=	{ "LDA", op_lda, mode_absolute,		0 }, // PASSED
	[0xBD]	=	{ "LDA", op_lda, mode_absolute_x,	0 }, // PASSED
	[0xB9]	=	{ "LDA", op_lda, mode_absolute_y,	0 }, // PASSED
	[0xA1]	=	{ "LDA", op_lda, mode_indirect_x,	0 }, // PASSED
	[0xB1]	=	{ "LDA", op_lda, mode_indirect_y, 	0 }, // PASSED
	[0xA2]	=	{ "LDX", op_ldx, mode_immediate, 	0 }, // PASSED
	[0xA6]	=	{ "LDX", op_ldx, mode_zero_page, 	0 }, // PASSED
	[0xB6]	=	{ "LDX", op_ldx, mode_zero_page_y,	0 }, // PASSED
	[0xAE]	=	{ "LDX", op_ldx, mode_absolute,		0 }, // PASSED
	[0xBE]	=	{ "LDX", op_ldx, mode_absolute_y,	0 }, // PASSED
	[0xA0]	=	{ "LDY", op_ldy, mode_immediate,	0 }, // PASSED
	[0xA4]	=	{ "LDY", op_ldy, mode_zero_page,	0 }, // PASSED
	[0xB4]	=	{ "LDY", op_ldy, mode_zero_page_x, 	0 }, // PASSED
	[0xAC]	=	{ "LDY", op_ldy, mode_absolute,		0 }, // PASSED
	[0xBC]	=	{ "LDY", op_ldy, mode_absolute_x,	0 }, // PASSED
	[0x85]	=	{ "STA", op_sta, mode_zero_page,	1 }, // PASSED
	[0x95]	=	{ "STA", op_sta, mode_zero_page_x,	1 }, // PASSED
	[0x8D]	=	{ "STA", op_sta, mode_absolute,		1 }, // PASSED
	[0x9D]	=	{ "STA", op_sta, mode_absolute_x,	1 }, // PASSED
	[0x99]	=	{ "STA", op_sta, mode_absolute_y,	1 }, // PASSED
	[0x81]	=	{ "STA", op_sta, mode_indirect_x,	1 }, // PASSED
	[0x91]	=	{ "STA", op_sta, mode_indirect_y,	1 }, // PASSED
	[0x86]	=	{ "STX", op_stx, mode_zero_page,	1 }, // PASSED
	[0x96]	=	{ "STX", op_stx, mode_zero_page_y,	1 }, // PASSED
	[0x8E]	=	{ "STX", op_stx, mode_absolute,		1 }, // PASSED
	[0x84]	=	{ "STY", op_sty, mode_zero_page,	1 }, // PASSED
	[0x94]	=	{ "STY", op_sty, mode_zero_page_x,	1 }, // PASSED
	[0x8C]	=	{ "STY", op_sty, mode_absolute,		1 }, // PASSED
	[0xAA]	=	{ "TAX", op_tax, mode_implied,		0 }, // PASSED
	[0xA8]	=	{ "TAY", op_tay, mode_implied,		0 }, // PASSED
	[0xBA]	=	{ "TSX", op_tsx, mode_implied, 		0 }, // PASSED
	[0x8A]	=	{ "TXA", op_txa, mode_implied, 		0 }, // PASSED
	[0x9A]	=	{ "TXS", op_txs, mode_implied,		0 }, // PASSED
	[0x98]	=	{ "TYA", op_tya, mode_implied,		0 }, // PASSED
	[0x69]	=	{ "ADC", op_adc, mode_immediate,	0 }, // PASSED
	[0x65]	=	{ "ADC", op_adc, mode_zero_page,	0 }, // PASSED
	[0x75]	=	{ "ADC", op_adc, mode_zero_page_x,	0 }, // PASSED
	[0x6D]	=	{ "ADC", op_adc, mode_absolute,		0 }, // PASSED
	[0x7D]	=	{ "ADC", op_adc, mode_absolute_x,	0 }, // PASSED
	[0x79]	=	{ "ADC", op_adc, mode_absolute_y,	0 }, // PASSED
	[0x61]	=	{ "ADC", op_adc, mode_indirect_x,	0 }, // PASSED 
	[0x71]	=	{ "ADC", op_adc, mode_indirect_y,	0 }, // PASSED 
	[0xE9]	=	{ "SBC", op_sbc, mode_immediate,	0 }, // PASSED
	[0xE5]	=	{ "SBC", op_sbc, mode_zero_page,	0 }, // PASSED
	[0xF5]	=	{ "SBC", op_sbc, mode_zero_page_x,	0 }, // PASSED
	[0xED]	=	{ "SBC", op_sbc, mode_absolute,		0 }, // PASSED
	[0xFD]	=	{ "SBC", op_sbc, mode_absolute_x,	0 }, // PASSED
	[0xF9]	=	{ "SBC", op_sbc, mode_absolute_y, 	0 }, // PASSED
	[0xE1]	=	{ "SBC", op_sbc, mode_indirect_x,	0 }, // PASSED
	[0xF1]	=	{ "SBC", op_sbc, mode_indirect_y,	0 }, // PASSED
	[0xE6]	=	{ "INC", op_inc, mode_zero_page,	1 }, // PASSED
	[0xF6]	=	{ "INC", op_inc, mode_zero_page_x,	1 }, // PASSED
	[0xEE]	=	{ "INC", op_inc, mode_absolute,		1 }, // PASSED
	[0xFE]	=	{ "INC", op_inc, mode_absolute_x,	1 }, // PASSED
	[0xC6]	=	{ "DEC", op_dec, mode_zero_page, 	1 }, // PASSED
	[0xD6]	=	{ "DEC", op_dec, mode_zero_page_x,	1 }, // PASSED
	[0xCE]	=	{ "DEC", op_dec, mode_absolute,		1 }, // PASSED
	[0xDE]	=	{ "DEC", op_dec, mode_absolute_x,	1 }, // PASSED
};



// >> BUS <<

static bits_8 processor_read(processor_6502 * processor, uint16_t address)
{
	return processor->bus.read_fn((void *) processor->bus.infra, address);
}
static void processor_write(processor_6502 * processor, uint16_t address, uint8_t data)
{
	processor->bus.write_fn((void *) processor->bus.infra, address, data);
}

// >> OTHERS <<

static uint16_t little_endian_solver(uint16_t low, uint16_t high, uint8_t add, uint16_t *carry)
{
	*carry = ((low + add) & 0x100);
	return ( (high << 8) | ((low + add) & 0xFF) );
}

static uint8_t oops(uint8_t force_oops, uint8_t condition)
{
	return (uint8_t)(force_oops || condition);
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

static bits_16 mode_immediate(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 addr 	= processor->reg_pc++;
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_zero_page(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 addr 	= processor_read(processor, processor->reg_pc++);
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_zero_page_x(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 addr 	= processor_read(processor, processor->reg_pc++);
	(void) processor_read(processor, addr); 			// Dummy Read
	addr 		= (addr + processor->reg_x) & 0xFF;
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_zero_page_y(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 addr 	= processor_read(processor, processor->reg_pc++);
	(void) processor_read(processor, addr); 			// Dummy Read
	addr 		= (addr + processor->reg_y) & 0xFF;
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_absolute(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 low 	= processor_read(processor, processor->reg_pc++);
	bits_16 high 	= processor_read(processor, processor->reg_pc++);
	bits_16 carry 	= 0;						// Ignore Carry in this case
	bits_16 addr 	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_absolute_x(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 low 	= processor_read(processor, processor->reg_pc++);
	bits_16 high 	= processor_read(processor, processor->reg_pc++);
	bits_16 carry 	= 0;
	bits_16 addr 	= little_endian_solver(low, high, processor->reg_x, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) processor_read(processor, addr); 		// Dummy Read
		addr 	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static bits_16 mode_absolute_y(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 low 	= processor_read(processor, processor->reg_pc++);
	bits_16 high 	= processor_read(processor, processor->reg_pc++);
	bits_16 carry	= 0;
	bits_16 addr	= little_endian_solver(low, high, processor->reg_y, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) processor_read(processor, addr);			// Dummy Read
		addr	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static bits_16 mode_indirect_x(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 l_addr	= processor_read(processor, processor->reg_pc++);
	(void) processor_read(processor, l_addr);			// Dummy Read
	l_addr 		= (l_addr + processor->reg_x) & 0xFF;
	bits_16 low	= processor_read(processor, l_addr);
	bits_16 high	= processor_read(processor, (l_addr + 1) & 0xFF);
	bits_16 carry	= 0;						// Ignore Carry in this case.
	bits_16 addr 	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
	return addr;
}

static bits_16 mode_indirect_y(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 l_addr 	= processor_read(processor, processor->reg_pc++);
	bits_16 low 	= processor_read(processor, l_addr);
	bits_16 high 	= processor_read(processor, (l_addr + 1) & 0xFF);
	bits_16 carry	= 0;
	bits_16 addr 	= little_endian_solver(low, high, processor->reg_y, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) processor_read(processor, addr);			// Dummy Read
		addr	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static bits_16 mode_implied(processor_6502 * processor, bits_8 force_oops)
{
	(void) processor_read(processor, processor->reg_pc);
	UNUSED(force_oops);
	return processor->reg_pc;
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

static void op_lda(processor_6502 * processor, bits_16 addr)
{
	processor->reg_a = processor_read(processor, addr);
	set_nz(processor, processor->reg_a);
}

static void op_ldx(processor_6502 * processor, bits_16 addr)
{
	processor->reg_x = processor_read(processor, addr);
	set_nz(processor, processor->reg_x);
}

static void op_ldy(processor_6502 * processor, bits_16 addr)
{
	processor->reg_y = processor_read(processor, addr);
	set_nz(processor, processor->reg_y);
}

static void op_sta(processor_6502 * processor, bits_16 addr)
{
	processor_write(processor, addr, processor->reg_a);
}

static void op_stx(processor_6502 * processor, bits_16 addr)
{
	processor_write(processor, addr, processor->reg_x);
}

static void op_sty(processor_6502 * processor, bits_16 addr)
{
	processor_write(processor, addr, processor->reg_y);
}

static void op_tax(processor_6502 * processor, bits_16 addr)
{
	processor->reg_x = processor->reg_a;
	set_nz(processor, processor->reg_x);
	UNUSED(addr);
}

static void op_tay(processor_6502 * processor, bits_16 addr)
{
	processor->reg_y = processor->reg_a;
	set_nz(processor, processor->reg_y);
	UNUSED(addr);
}

static void op_tsx(processor_6502 * processor, bits_16 addr)
{
	processor->reg_x = processor->reg_sp;
	set_nz(processor, processor->reg_x);
	UNUSED(addr);
}

static void op_txa(processor_6502 * processor, bits_16 addr)
{
	processor->reg_a = processor->reg_x;
	set_nz(processor, processor->reg_a);
	UNUSED(addr);
}

static void op_txs(processor_6502 * processor, bits_16 addr)
{
	processor->reg_sp = processor->reg_x;
	UNUSED(addr);
}

static void op_tya(processor_6502 * processor, bits_16 addr)
{
	processor->reg_a = processor->reg_y;
	set_nz(processor, processor->reg_a);
	UNUSED(addr);
}

static void op_adc(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	if (processor->reg_p & FLAG_DEC)
		handle_op_adc_decimal_mode(processor, data);
	else
		handle_op_adc_binary_mode(processor, data);
}

static void op_sbc(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	if (processor->reg_p & FLAG_DEC)
		handle_op_sbc_decimal_mode(processor, data);
	else
		handle_op_sbc_binary_mode(processor, data);
}

static void op_inc(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor_write(processor, addr, data);
	data++;
	processor_write(processor, addr, data);
	set_nz(processor, data);
}

static void op_dec(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor_write(processor, addr, data);
	data--;
	processor_write(processor, addr, data);
	set_nz(processor, data);
}



// >> PUBLIC FUNCTIONS <<

void init_6502(processor_6502 * processor)
{	
	processor->reg_a  	= 0x00;
	processor->reg_x  	= 0x00;
	processor->reg_y  	= 0x00;
	processor->reg_sp 	= 0xFD;
	processor->reg_p 	= 0x24;
	processor->read_write	= 0x00;
	processor->reg_pc 	= 0x8000;
	processor->cycles 	= 0x00;	
}

void execute_opcode(processor_6502 * processor, memory_16 * memory)
{	
	bits_8 opcode = processor_read(processor, processor->reg_pc++);
	opcode_entry execute = op_table[opcode];

	if (execute.op == NULL) {
		printf("UNDEFINED OPCODE...\n");
		return;
	}
//	printf("Executing %s\n", execute.op_name);
	bits_16 address = execute.mode(processor, execute.force_oops);
	execute.op(processor, address);
//	printf(">> A:%02x  X:%02x  Y:%02x  P:%02x  Cycles: %016lx\n", processor->reg_a, processor->reg_x, processor->reg_y, processor->reg_p, processor->cycles);
	UNUSED(memory);
}

