#include "./../../include/hardwares/processor_6502.h"
#include "./../../include/buses.h"
#include "./../../include/hardwares/memory_16.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Status Pointer Flags
#define FLAG_NEG		0x80		// Negative
#define FLAG_OVF		0x40		// Overflow
#define FLAG_NOEFFECT		0x20		// Literally no effect on the processor
#define FLAG_BRK		0x10		// Break
#define FLAG_DEC		0x08		// Decimal
#define FLAG_INT		0x04		// Interrupt
#define FLAG_ZER		0x02		// Zero
#define FLAG_CAR		0x01		// Carry
#define FLAG_INITIAL_STATE	0x24		// Only NOEFFECT and INT flags are set

// Interrupts Addresses
#define INTERRUPT_IRQ_LW_ADDR	0xFFFE		// IRQ/BRK Interrupt
#define INTERRUPT_NMI_LW_ADDR	0xFFFA		// NMI Interrupt
#define INTERRUPT_RESET_LW_ADDR	0xFFFC		// RESET Interrupt
#define IRQ_LOW_VALUE		0
#define IRQ_HIGH_VALUE		1
#define NMI_LOW_VALUE		0
#define NMI_HIGH_VALUE		1

// Stack Pointer Start Index
#define ADDR_STK		0x100

// Others
#define NUM_OPCODES		0x100		// Total amount of OPCODES
#define UNUSED(x)		(void)(x)	// Unused variables
#define TRUE			1
#define FALSE			0




// Function Pointers to handle Addressing Modes and Opcodes.
typedef bits_16	(*addr_fn) (processor_6502 * processor, bits_8 force_oops);
typedef void	(*op_fn) (processor_6502 * processor, bits_16 addr);

// Description: Opcode Structure.
typedef struct {
	const char	*op_name;
	const char	*mode_name;
	op_fn		op;
	addr_fn		mode;
	bits_8		force_oops; // Force Dummy Cycles;
} opcode_entry;

// Bus
static bits_8	processor_read(processor_6502 * processor, uint16_t address);
static void	processor_write(processor_6502 * processor, uint16_t address, uint8_t data);

// Addressing Mode Functions
static bits_16 mode_immediate(processor_6502 * processor, bits_8 force_oops);	// MODE 00
static bits_16 mode_zero_page(processor_6502 * processor, bits_8 force_oops);	// MODE 01
static bits_16 mode_zero_page_x(processor_6502 * processor, bits_8 force_oops); // MODE 02
static bits_16 mode_zero_page_y(processor_6502 * processor, bits_8 force_oops);	// MODE 03
static bits_16 mode_absolute(processor_6502 * processor, bits_8 force_oops);	// MODE 04
static bits_16 mode_absolute_x(processor_6502 * processor, bits_8 force_oops);	// MODE 05
static bits_16 mode_absolute_y(processor_6502 * processor, bits_8 force_oops);	// MODE 06
static bits_16 mode_indirect(processor_6502 * processor, bits_8 force_oops);	// MODE 07
static bits_16 mode_indirect_x(processor_6502 * processor, bits_8 force_oops);	// MODE 08
static bits_16 mode_indirect_y(processor_6502 * processor, bits_8 force_oops);	// MODE 09
static bits_16 mode_implied(processor_6502 * processor, bits_8 force_oops);	// MODE 10
static bits_16 mode_relative(processor_6502 * processor, bits_8 force_oops);	// MODE 11

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
static void op_txa(processor_6502 * processor, bits_16 addr);
static void op_tya(processor_6502 * processor, bits_16 addr);
// Arithmetic:
static void op_adc(processor_6502 * processor, bits_16 addr);
static void op_sbc(processor_6502 * processor, bits_16 addr);
static void op_inc(processor_6502 * processor, bits_16 addr);
static void op_dec(processor_6502 * processor, bits_16 addr);
static void op_inx(processor_6502 * processor, bits_16 addr);
static void op_dex(processor_6502 * processor, bits_16 addr);
static void op_iny(processor_6502 * processor, bits_16 addr);
static void op_dey(processor_6502 * processor, bits_16 addr);
// Shift:
static void op_asl(processor_6502 * processor, bits_16 addr);
static void op_lsr(processor_6502 * processor, bits_16 addr);
static void op_rol(processor_6502 * processor, bits_16 addr);
static void op_ror(processor_6502 * processor, bits_16 addr);
static void op_asl_accum(processor_6502 * processor, bits_16 addr);
static void op_lsr_accum(processor_6502 * processor, bits_16 addr);
static void op_rol_accum(processor_6502 * processor, bits_16 addr);
static void op_ror_accum(processor_6502 * processor, bits_16 addr);
// Bitwise:
static void op_and(processor_6502 * processor, bits_16 addr);
static void op_ora(processor_6502 * processor, bits_16 addr);
static void op_eor(processor_6502 * processor, bits_16 addr);
static void op_bit(processor_6502 * processor, bits_16 addr);
// Compare:
static void op_cmp(processor_6502 * processor, bits_16 addr);
static void op_cpx(processor_6502 * processor, bits_16 addr);
static void op_cpy(processor_6502 * processor, bits_16 addr);
// Branch:
static void op_bcc(processor_6502 * processor, bits_16 addr);
static void op_bcs(processor_6502 * processor, bits_16 addr);
static void op_beq(processor_6502 * processor, bits_16 addr);
static void op_bne(processor_6502 * processor, bits_16 addr);
static void op_bpl(processor_6502 * processor, bits_16 addr);
static void op_bmi(processor_6502 * processor, bits_16 addr);
static void op_bvc(processor_6502 * processor, bits_16 addr);
static void op_bvs(processor_6502 * processor, bits_16 addr);
// Jump:
static void op_jmp(processor_6502 * processor, bits_16 addr);
static void op_jsr(processor_6502 * processor, bits_16 addr);
static void op_rts(processor_6502 * processor, bits_16 addr);
static void op_brk(processor_6502 * processor, bits_16 addr);
static void op_rti(processor_6502 * processor, bits_16 addr);
// Stack
static void op_pha(processor_6502 * processor, bits_16 addr);
static void op_pla(processor_6502 * processor, bits_16 addr);
static void op_php(processor_6502 * processor, bits_16 addr);
static void op_plp(processor_6502 * processor, bits_16 addr);
static void op_tsx(processor_6502 * processor, bits_16 addr);
static void op_txs(processor_6502 * processor, bits_16 addr);
// Flags
static void op_clc(processor_6502 * processor, bits_16 addr);
static void op_sec(processor_6502 * processor, bits_16 addr);
static void op_cli(processor_6502 * processor, bits_16 addr);
static void op_sei(processor_6502 * processor, bits_16 addr);
static void op_cld(processor_6502 * processor, bits_16 addr);
static void op_sed(processor_6502 * processor, bits_16 addr);
static void op_clv(processor_6502 * processor, bits_16 addr);
// Other
static void op_nop(processor_6502 * processor, bits_16 addr);


// Flags Management
static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition);
static void set_nz(processor_6502 * processor, bits_8 data);

// Stack Management
static void 	stack_push(processor_6502 * processor, uint8_t data);
static uint8_t 	stack_pull(processor_6502 * processor);
static uint8_t 	stack_read(processor_6502 * processor, uint8_t stack);

// Others
static uint16_t	little_endian_solver(uint16_t low, uint16_t high, uint8_t add, uint16_t *carry);
static uint8_t 	oops(uint8_t force_oops, uint8_t condition);
static void	nmi_request_store_new_value(processor_6502 * processor);
static void	response_interrupt_request_if_any(processor_6502 * processor);

// Handles (Helpers)
static void handle_interrupt_irq_nmi(processor_6502 * processor, uint16_t interrupt_lw_addr, uint8_t brk_triggered);
//static void handle_interrupt_reset(processor_6502 * processor, uint8_t is_power_up);
static void handle_op_adc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_adc_binary_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_decimal_mode(processor_6502 * p, uint8_t data);
static void handle_op_sbc_binary_mode(processor_6502 * p, uint8_t data);
static void handle_branch(processor_6502 * processor, uint16_t addr, uint8_t condition);


/*
 * Description: A table containing all OPCODES
 */
static const opcode_entry op_table[NUM_OPCODES] = {
	[0xA9]	=	{ "LDA", "Immediate",		op_lda, 	mode_immediate, 	0 }, // PASSED
	[0xA5]	=	{ "LDA", "Zero Page",		op_lda, 	mode_zero_page, 	0 }, // PASSED
	[0xB5]	=	{ "LDA", "Zero Page,X",		op_lda, 	mode_zero_page_x, 	0 }, //	PASSED
	[0xAD]	=	{ "LDA", "Absolute",		op_lda, 	mode_absolute,		0 }, // PASSED
	[0xBD]	=	{ "LDA", "Absolute,X",		op_lda, 	mode_absolute_x,	0 }, // PASSED
	[0xB9]	=	{ "LDA", "Absolute,Y",		op_lda, 	mode_absolute_y,	0 }, // PASSED
	[0xA1]	=	{ "LDA", "(Indirect,X)",	op_lda, 	mode_indirect_x,	0 }, // PASSED
	[0xB1]	=	{ "LDA", "(Indirect),Y",	op_lda, 	mode_indirect_y, 	0 }, // PASSED
	[0xA2]	=	{ "LDX", "Immediate",		op_ldx, 	mode_immediate, 	0 }, // PASSED
	[0xA6]	=	{ "LDX", "Zero Page",		op_ldx, 	mode_zero_page, 	0 }, // PASSED
	[0xB6]	=	{ "LDX", "Zero Page,Y",		op_ldx, 	mode_zero_page_y,	0 }, // PASSED
	[0xAE]	=	{ "LDX", "Absolute",		op_ldx, 	mode_absolute,		0 }, // PASSED
	[0xBE]	=	{ "LDX", "Absolute,Y",		op_ldx, 	mode_absolute_y,	0 }, // PASSED
	[0xA0]	=	{ "LDY", "Immediate",		op_ldy, 	mode_immediate,		0 }, // PASSED
	[0xA4]	=	{ "LDY", "Zero Page",		op_ldy, 	mode_zero_page,		0 }, // PASSED
	[0xB4]	=	{ "LDY", "Zero Page,X",		op_ldy, 	mode_zero_page_x, 	0 }, // PASSED
	[0xAC]	=	{ "LDY", "Absolute",		op_ldy, 	mode_absolute,		0 }, // PASSED
	[0xBC]	=	{ "LDY", "Absolute,X",		op_ldy, 	mode_absolute_x,	0 }, // PASSED
	[0x85]	=	{ "STA", "Zero Page",		op_sta, 	mode_zero_page,		1 }, // PASSED
	[0x95]	=	{ "STA", "Zero Page,X",		op_sta, 	mode_zero_page_x,	1 }, // PASSED
	[0x8D]	=	{ "STA", "Absolute",		op_sta, 	mode_absolute,		1 }, // PASSED
	[0x9D]	=	{ "STA", "Absolute,X",		op_sta, 	mode_absolute_x,	1 }, // PASSED
	[0x99]	=	{ "STA", "Absolute,Y",		op_sta, 	mode_absolute_y,	1 }, // PASSED
	[0x81]	=	{ "STA", "(Indirect,X)",	op_sta, 	mode_indirect_x,	1 }, // PASSED
	[0x91]	=	{ "STA", "(Indirect),Y",	op_sta, 	mode_indirect_y,	1 }, // PASSED
	[0x86]	=	{ "STX", "Zero Page",		op_stx, 	mode_zero_page,		1 }, // PASSED
	[0x96]	=	{ "STX", "Zero Page,Y",		op_stx, 	mode_zero_page_y,	1 }, // PASSED
	[0x8E]	=	{ "STX", "Absolute",		op_stx, 	mode_absolute,		1 }, // PASSED
	[0x84]	=	{ "STY", "Zero Page",		op_sty, 	mode_zero_page,		1 }, // PASSED
	[0x94]	=	{ "STY", "Zero Page,X",		op_sty, 	mode_zero_page_x,	1 }, // PASSED
	[0x8C]	=	{ "STY", "Absolute",		op_sty, 	mode_absolute,		1 }, // PASSED
	[0xAA]	=	{ "TAX", "Implied",		op_tax, 	mode_implied,		0 }, // PASSED
	[0xA8]	=	{ "TAY", "Implied",		op_tay, 	mode_implied,		0 }, // PASSED
	[0xBA]	=	{ "TSX", "Implied",		op_tsx, 	mode_implied, 		0 }, // PASSED
	[0x8A]	=	{ "TXA", "Implied",		op_txa, 	mode_implied, 		0 }, // PASSED
	[0x9A]	=	{ "TXS", "Implied",		op_txs, 	mode_implied,		0 }, // PASSED
	[0x98]	=	{ "TYA", "Implied",		op_tya, 	mode_implied,		0 }, // PASSED
	[0x69]	=	{ "ADC", "Immediate",		op_adc, 	mode_immediate,		0 }, // PASSED
	[0x65]	=	{ "ADC", "Zero Page",		op_adc, 	mode_zero_page,		0 }, // PASSED
	[0x75]	=	{ "ADC", "Zero Page,X",		op_adc, 	mode_zero_page_x,	0 }, // PASSED
	[0x6D]	=	{ "ADC", "Absolute",		op_adc, 	mode_absolute,		0 }, // PASSED
	[0x7D]	=	{ "ADC", "Absolute,X",		op_adc, 	mode_absolute_x,	0 }, // PASSED
	[0x79]	=	{ "ADC", "Absolute,Y",		op_adc, 	mode_absolute_y,	0 }, // PASSED
	[0x61]	=	{ "ADC", "(Indirect,X)",	op_adc, 	mode_indirect_x,	0 }, // PASSED 
	[0x71]	=	{ "ADC", "(Indirect),Y",	op_adc, 	mode_indirect_y,	0 }, // PASSED 
	[0xE9]	=	{ "SBC", "Immediate",		op_sbc, 	mode_immediate,		0 }, // PASSED
	[0xE5]	=	{ "SBC", "Zero Page",		op_sbc, 	mode_zero_page,		0 }, // PASSED
	[0xF5]	=	{ "SBC", "Zero Page,X",		op_sbc, 	mode_zero_page_x,	0 }, // PASSED
	[0xED]	=	{ "SBC", "Absolute",		op_sbc, 	mode_absolute,		0 }, // PASSED
	[0xFD]	=	{ "SBC", "Absolute,X",		op_sbc, 	mode_absolute_x,	0 }, // PASSED
	[0xF9]	=	{ "SBC", "Absolute,Y",		op_sbc, 	mode_absolute_y, 	0 }, // PASSED
	[0xE1]	=	{ "SBC", "(Indirect,X)",	op_sbc, 	mode_indirect_x,	0 }, // PASSED
	[0xF1]	=	{ "SBC", "(Indirect),Y",	op_sbc, 	mode_indirect_y,	0 }, // PASSED
	[0xE6]	=	{ "INC", "Zero Page",		op_inc, 	mode_zero_page,		1 }, // PASSED
	[0xF6]	=	{ "INC", "Zero Page,X",		op_inc, 	mode_zero_page_x,	1 }, // PASSED
	[0xEE]	=	{ "INC", "Absolute",		op_inc, 	mode_absolute,		1 }, // PASSED
	[0xFE]	=	{ "INC", "Absolute,X",		op_inc, 	mode_absolute_x,	1 }, // PASSED
	[0xC6]	=	{ "DEC", "Zero Page",		op_dec, 	mode_zero_page, 	1 }, // PASSED
	[0xD6]	=	{ "DEC", "Zero Page,X",		op_dec, 	mode_zero_page_x,	1 }, // PASSED
	[0xCE]	=	{ "DEC", "Absolute",		op_dec, 	mode_absolute,		1 }, // PASSED
	[0xDE]	=	{ "DEC", "Absolute,X",		op_dec, 	mode_absolute_x,	1 }, // PASSED
	[0xE8]	=	{ "INX", "Implied",		op_inx, 	mode_implied,		0 }, // PASSED
	[0xCA]	=	{ "DEX", "Implied",		op_dex, 	mode_implied,		0 }, // PASSED
	[0xC8]	=	{ "INY", "Implied",		op_iny, 	mode_implied,		0 }, // PASSED
	[0x88]	=	{ "DEY", "Implied",		op_dey, 	mode_implied,		0 }, // PASSED
	[0x0A]	=	{ "ASL", "Implied",		op_asl_accum,	mode_implied,		1 }, // PASSED
	[0x06]	=	{ "ASL", "Zero Page",		op_asl,		mode_zero_page,		1 }, // PASSED
	[0x16]	=	{ "ASL", "Zero Page,X",		op_asl,		mode_zero_page_x,	1 }, // PASSED
	[0x0E]	=	{ "ASL", "Absolute",		op_asl, 	mode_absolute,		1 }, // PASSED
	[0x1E]	=	{ "ASL", "Absolute,X",		op_asl,		mode_absolute_x,	1 }, // PASSED
	[0x4A]	=	{ "LSR", "Implied",		op_lsr_accum,	mode_implied,		1 }, // PASSED
	[0x46]	=	{ "LSR", "Zero Page",		op_lsr,		mode_zero_page,		1 }, // PASSED
	[0x56]	=	{ "LSR", "Zero Page,X",		op_lsr,		mode_zero_page_x,	1 }, // PASSED
	[0x4E]	=	{ "LSR", "Absolute",		op_lsr,		mode_absolute,		1 }, // PASSED
	[0x5E]	=	{ "LSR", "Absolute,X",		op_lsr,		mode_absolute_x,	1 }, // PASSED
	[0x2A]	=	{ "ROL", "Implied",		op_rol_accum,	mode_implied,		1 }, // PASSED
	[0x26]	=	{ "ROL", "Zero Page",		op_rol,		mode_zero_page,		1 }, // PASSED
	[0x36]	=	{ "ROL", "Zero Page,X",		op_rol,		mode_zero_page_x,	1 }, // PASSED
	[0x2E]	=	{ "ROL", "Absolute",		op_rol,		mode_absolute,		1 }, // PASSED
	[0x3E]	=	{ "ROL", "Absolute,X",		op_rol,		mode_absolute_x,	1 }, // PASSED
	[0x6A]	=	{ "ROR", "Implied",		op_ror_accum,	mode_implied,		1 }, // PASSED
	[0x66]	=	{ "ROR", "Zero Page",		op_ror,		mode_zero_page,		1 }, // PASSED
	[0x76]	=	{ "ROR", "Zero Page,X",		op_ror,		mode_zero_page_x,	1 }, // PASSED
	[0x6E]	=	{ "ROR", "Absolute",		op_ror,		mode_absolute,		1 }, // PASSED
	[0x7E]	=	{ "ROR", "Absolute,X",		op_ror,		mode_absolute_x,	1 }, // PASSED
	[0x29]	=	{ "AND", "Immediate",		op_and,		mode_immediate,		0 }, // PASSED
	[0x25]	=	{ "AND", "Zero Page",		op_and,		mode_zero_page,		0 }, // PASSED
	[0x35]	=	{ "AND", "Zero Page,X",		op_and,		mode_zero_page_x,	0 }, // PASSED
	[0x2D]	=	{ "AND", "Absolute",		op_and,		mode_absolute,		0 }, // PASSED
	[0x3D]	=	{ "AND", "Absolute,X",		op_and,		mode_absolute_x,	0 }, // PASSED
	[0x39]	=	{ "AND", "Absolute,Y",		op_and,		mode_absolute_y,	0 }, // PASSED
	[0x21]	=	{ "AND", "(Indirect,X)",	op_and,		mode_indirect_x,	0 }, // PASSED
	[0x31]	=	{ "AND", "(Indirect),Y",	op_and,		mode_indirect_y,	0 }, // PASSED
	[0x09]	=	{ "ORA", "Immediate",		op_ora,		mode_immediate,		0 }, // PASSED
	[0x05]	=	{ "ORA", "Zero Page",		op_ora,		mode_zero_page,		0 }, // PASSED
	[0x15]	=	{ "ORA", "Zero Page,X",		op_ora,		mode_zero_page_x,	0 }, // PASSED
	[0x0D]	=	{ "ORA", "Absolute",		op_ora,		mode_absolute,		0 }, // PASSED
	[0x1D]	=	{ "ORA", "Absolute,X",		op_ora,		mode_absolute_x,	0 }, // PASSED
	[0x19]	=	{ "ORA", "Absolute,Y",		op_ora,		mode_absolute_y,	0 }, // PASSED
	[0x01]	=	{ "ORA", "(Indirect,X)",	op_ora,		mode_indirect_x,	0 }, // PASSED
	[0x11]	=	{ "ORA", "(Indirect),Y",	op_ora,		mode_indirect_y,	0 }, // PASSED
	[0x49]	=	{ "EOR", "Immediate",		op_eor,		mode_immediate,		0 }, // PASSED
	[0x45]	=	{ "EOR", "Zero Page",		op_eor,		mode_zero_page,		0 }, // PASSED
	[0x55]	=	{ "EOR", "Zero Page,X",		op_eor,		mode_zero_page_x,	0 }, // PASSED
	[0x4D]	=	{ "EOR", "Absolute",		op_eor,		mode_absolute,		0 }, // PASSED
	[0x5D]	=	{ "EOR", "Absolute,X",		op_eor,		mode_absolute_x,	0 }, // PASSED
	[0x59]	=	{ "EOR", "Absolute,Y",		op_eor,		mode_absolute_y,	0 }, // PASSED
	[0x41]	=	{ "EOR", "(Indirect,X)",	op_eor,		mode_indirect_x,	0 }, // PASSED
	[0x51]	=	{ "EOR", "(Indirect),Y",	op_eor,		mode_indirect_y,	0 }, // PASSED
	[0x24]	=	{ "BIT", "Zero Page",		op_bit,		mode_zero_page,		0 }, // PASSED
	[0x2C]	=	{ "BIT", "Absolute",		op_bit,		mode_absolute,		0 }, // PASSED
	[0xC9]	=	{ "CMP", "Immediate",		op_cmp,		mode_immediate,		0 }, // PASSED
	[0xC5]	=	{ "CMP", "Zero Page",		op_cmp,		mode_zero_page,		0 }, // PASSED
	[0xD5]	=	{ "CMP", "Zero Page,X",		op_cmp,		mode_zero_page_x,	0 }, // PASSED
	[0xCD]	=	{ "CMP", "Absolute",		op_cmp,		mode_absolute,		0 }, // PASSED
	[0xDD]	=	{ "CMP", "Absolute,X",		op_cmp,		mode_absolute_x,	0 }, // PASSED
	[0xD9]	=	{ "CMP", "Absolute,Y",		op_cmp,		mode_absolute_y,	0 }, // PASSED
	[0xC1]	=	{ "CMP", "(Indirect,X)",	op_cmp,		mode_indirect_x,	0 }, // PASSED
	[0xD1]	=	{ "CMP", "(Indirect),Y",	op_cmp,		mode_indirect_y,	0 }, // PASSED
	[0xE0]	=	{ "CPX", "Immediate",		op_cpx,		mode_immediate,		0 }, // PASSED
	[0xE4]	=	{ "CPX", "Zero Page",		op_cpx,		mode_zero_page,		0 }, // PASSED
	[0xEC]	=	{ "CPX", "Absolute",		op_cpx,		mode_absolute,		0 }, // PASSED
	[0xC0]	=	{ "CPY", "Immediate",		op_cpy,		mode_immediate,		0 }, // PASSED
	[0xC4]	=	{ "CPY", "Zero Page",		op_cpy,		mode_zero_page,		0 }, // PASSED
	[0xCC]	=	{ "CPY", "Absolute",		op_cpy,		mode_absolute,		0 }, // PASSED
	[0x90]	=	{ "BCC", "Relative",		op_bcc,		mode_relative,		0 }, // PASSED
	[0xB0]	=	{ "BCS", "Relative",		op_bcs,		mode_relative,		0 }, // PASSED
	[0xF0]	=	{ "BEQ", "Relative",		op_beq,		mode_relative,		0 }, // PASSED
	[0xD0]	=	{ "BNE", "Relative",		op_bne,		mode_relative,		0 }, // PASSED
	[0x10]	=	{ "BPL", "Relative",		op_bpl,		mode_relative,		0 }, // PASSED
	[0x30]	=	{ "BMI", "Relative",		op_bmi,		mode_relative,		0 }, // PASSED
	[0x50]	=	{ "BVC", "Relative",		op_bvc,		mode_relative,		0 }, // PASSED
	[0x70]	=	{ "BVS", "Relative",		op_bvs,		mode_relative,		0 }, // PASSED
	[0x4C]	=	{ "JMP", "Absolute",		op_jmp,		mode_absolute,		0 }, // PASSED
	[0x6C]	=	{ "JMP", "(Indirect)",		op_jmp,		mode_indirect,		0 }, // PASSED
	[0x20]	=	{ "JSR", "Absolute",		op_jsr,		mode_relative,		0 }, //	PASSED	Addressing Mode Absolute Hardcoded into the op_jsr.
	[0x60]	=	{ "RTS", "Implied",		op_rts,		mode_implied,		0 }, // PASSED
	[0x00]	=	{ "BRK", "Immediate",		op_brk,		mode_immediate,		0 }, // PASSED
	[0x40]	=	{ "RTI", "Implied",		op_rti,		mode_implied,		0 }, // PASSED
	[0x48]	=	{ "PHA", "Implied",		op_pha,		mode_implied,		0 }, // PASSED
	[0x68]	=	{ "PLA", "Implied",		op_pla,		mode_implied,		0 }, // PASSED
	[0x08]	=	{ "PHP", "Implied",		op_php,		mode_implied,		0 }, // PASSED
	[0x28]	=	{ "PLP", "Implied",		op_plp,		mode_implied,		0 }, // PASSED
	[0x18]	=	{ "CLC", "Implied",		op_clc,		mode_implied,		0 }, // PASSED
	[0x38]	=	{ "SEC", "Implied",		op_sec,		mode_implied,		0 }, // PASSED
	[0x58]	=	{ "CLI", "Implied",		op_cli,		mode_implied,		0 }, // PASSED
	[0x78]	=	{ "SEI", "Implied",		op_sei,		mode_implied,		0 }, // PASSED
	[0xD8]	=	{ "CLD", "Implied",		op_cld,		mode_implied,		0 }, // PASSED
	[0xF8]	=	{ "SED", "Implied",		op_sed,		mode_implied,		0 }, // PASSES
	[0xB8]	=	{ "CLV", "Implied",		op_clv,		mode_implied,		0 }, // PASSED
	[0xEA]	=	{ "NOP", "Implied",		op_nop,		mode_implied,		0 }, // PASSED
};






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

static void nmi_request_store_new_value(processor_6502 * processor)
{
	if ( (processor->nmi_previous == NMI_HIGH_VALUE) && (processor->nmi_pin == NMI_LOW_VALUE) )
		processor->nmi_request = 1;

	processor->nmi_previous = processor->nmi_pin;
}

static void response_interrupt_request_if_any(processor_6502 * processor)
{
	if (processor->nmi_request) {
		printf("NMI\n");
		handle_interrupt_irq_nmi(processor, INTERRUPT_NMI_LW_ADDR, 0);
		processor->nmi_request = 0;
	}
	
	if ( (processor->irq_pin == IRQ_LOW_VALUE) && ((processor->reg_p & FLAG_INT) == 0) ) {
		printf("IRQ\n");
		handle_interrupt_irq_nmi(processor, INTERRUPT_IRQ_LW_ADDR, 0);
		//processor->irq_pin = IRQ_HIGH_VALUE;
	}
}






// >> BUS <<

static bits_8 processor_read(processor_6502 * processor, uint16_t address)
{
	uint8_t data = processor->bus.read_fn((void *) processor->bus.infra, address);
	nmi_request_store_new_value(processor);
	return data;
}
static void processor_write(processor_6502 * processor, uint16_t address, uint8_t data)
{
	processor->bus.write_fn((void *) processor->bus.infra, address, data);
	nmi_request_store_new_value(processor);
}






// >> FLAGS <<

static void set_flag(processor_6502 * processor, bits_8 flag_mask, int condition)
{
	if (condition)
		processor->reg_p |= flag_mask;
	else
		processor->reg_p &= (~flag_mask);
}

static void set_nz(processor_6502 * processor, bits_8 data)
{
	set_flag(processor, FLAG_ZER, (data == 0));
	set_flag(processor, FLAG_NEG, ((data & FLAG_NEG) > 0));
}






// >> STACKS <<

static void stack_push(processor_6502 * processor, uint8_t data)
{
	processor_write(processor, ADDR_STK + processor->reg_sp, data);
	processor->reg_sp = (processor->reg_sp - 1) & 0xFF;
}

static uint8_t stack_pull(processor_6502 * processor)
{
	processor->reg_sp = (processor->reg_sp + 1) & 0xFF;
	return processor_read(processor, ADDR_STK + processor->reg_sp);
}

static bits_8 stack_read(processor_6502 * processor, uint8_t stack)
{
	return processor_read(processor, ADDR_STK + stack);
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

static bits_16 mode_indirect(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 p_low	= processor_read(processor, processor->reg_pc++) & 0xFF;
	bits_16 p_high	= processor_read(processor, processor->reg_pc++) & 0xFF;
	bits_16 carry	= 0;
	bits_16 low_addr= little_endian_solver(p_low, p_high, 0, &carry);
	bits_16 low	= processor_read(processor, low_addr);
	bits_16 high 	= 0;
	// CPU BUG
	if ( ((low_addr & 0x00FF) + 1) > 0x00FF )
		high	= processor_read(processor, low_addr & 0xFF00);
	else
		high	= processor_read(processor, (low_addr + 1) & 0xFFFF);
	bits_16 addr	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
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

static bits_16 mode_relative(processor_6502 * processor, bits_8 force_oops)
{
	bits_16 addr	= processor->reg_pc++;
	UNUSED(force_oops);
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

static void handle_interrupt_irq_nmi(processor_6502 * processor, uint16_t interrupt_lw_addr, uint8_t brk_triggered)
{
	if (brk_triggered && (interrupt_lw_addr == INTERRUPT_IRQ_LW_ADDR)) {
		(void) processor_read(processor, ((uint16_t)(processor->reg_pc - 1) & 0xFFFF));	//Force another read
		set_flag(processor, FLAG_BRK, 1);						// Just for the stack
	} else {
		(void) processor_read(processor, processor->reg_pc);				// Force an OPCODE read
		(void) processor_read(processor, processor->reg_pc + 1);			// Force another read
		set_flag(processor, FLAG_BRK, 0);
	}

	bits_8	high_pc		= (uint8_t)((processor->reg_pc & 0xFF00) >> 8);
	bits_8	low_pc		= (uint8_t)(processor->reg_pc & 0x00FF);
	stack_push(processor, high_pc);
	stack_push(processor, low_pc);
	stack_push(processor, processor->reg_p);
	set_flag(processor, FLAG_BRK, 0);					// Make break flag unset
	set_flag(processor, FLAG_INT, 1);					// Set Interrupt Disabled
	bits_16	low_addr	= processor_read(processor, interrupt_lw_addr);
	bits_16 high_addr	= processor_read(processor, (interrupt_lw_addr + 1) & 0xFFFF);
	bits_16 carry		= 0;
	processor->reg_pc	= little_endian_solver(low_addr, high_addr, 0, &carry);

}

static void handle_branch(processor_6502 * processor, uint16_t addr, uint8_t condition)
{
	int8_t relat 	= processor_read(processor, addr);
	uint16_t low 	= processor->reg_pc & 0x00FF;
	uint16_t high 	= processor->reg_pc & 0xFF00;
	uint16_t low_buf= low;
	if ( condition ) {
		(void) processor_read(processor, processor->reg_pc);
		low = (low + (int8_t)relat) & 0x00FF;
		processor->reg_pc = high | low;
		if ((low_buf + (int8_t)relat) > 0x00FF) {
			processor_read(processor, processor->reg_pc);
			processor->reg_pc += 0x100;
		}
		else if ((low_buf + (int8_t)relat) < 0) {
			processor_read(processor, processor->reg_pc);
			processor->reg_pc -= 0x100;
		}	
	}
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

static void op_inx(processor_6502 * processor, bits_16 addr)
{
	processor->reg_x++;
	set_nz(processor, processor->reg_x);
	UNUSED(addr);
}

static void op_dex(processor_6502 * processor, bits_16 addr)
{
	processor->reg_x--;
	set_nz(processor, processor->reg_x);
	UNUSED(addr);
}

static void op_iny(processor_6502 * processor, bits_16 addr)
{
	processor->reg_y++;
	set_nz(processor, processor->reg_y);
	UNUSED(addr);
}

static void op_dey(processor_6502 * processor, bits_16 addr)
{
	processor->reg_y--;
	set_nz(processor, processor->reg_y);
	UNUSED(addr);
}

static void op_asl_accum(processor_6502 * processor, bits_16 addr)
{
	uint8_t carry = ((processor->reg_a & 0b10000000) >> 7); // Get the last bit
	processor->reg_a = processor->reg_a << 1;
	set_nz(processor, processor->reg_a);
	set_flag(processor, FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_asl(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	uint8_t carry = ((data & 0b10000000) >> 7); // Get the last bit
	processor_write(processor, addr, data);
	data = data << 1;
	processor_write(processor, addr, data);
	set_nz(processor, data);
	set_flag(processor, FLAG_CAR, carry);
}

static void op_lsr_accum(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_CAR, processor->reg_a & FLAG_CAR);
	processor->reg_a = processor->reg_a >> 1;
	set_nz(processor, processor->reg_a);
	UNUSED(addr);
}

static void op_lsr(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor_write(processor, addr, data);
	set_flag(processor, FLAG_CAR, data & FLAG_CAR);
	data = data >> 1;
	processor_write(processor, addr, data);
	set_nz(processor, data);
}

static void op_rol_accum(processor_6502 * processor, bits_16 addr)
{
	uint8_t carry = ((processor->reg_a & 0b10000000) >> 7); // Get the last bit
	processor->reg_a = ((processor->reg_a << 1) | (processor->reg_p & FLAG_CAR));
	set_nz(processor, processor->reg_a);
	set_flag(processor, FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_rol(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
 	uint8_t carry = ((data & 0b10000000) >> 7); // Get the last bit
	processor_write(processor, addr, data);
	data = ((data << 1) | (processor->reg_p & FLAG_CAR));
	processor_write(processor, addr, data);
	set_nz(processor, data);
	set_flag(processor, FLAG_CAR, carry);
}

static void op_ror_accum(processor_6502 * processor, bits_16 addr)
{
	uint8_t carry = ((processor->reg_a & FLAG_CAR));
	processor->reg_a = ((processor->reg_a >> 1) | ((processor->reg_p & FLAG_CAR) << 7));
	set_nz(processor, processor->reg_a);
	set_flag(processor, FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_ror(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	uint8_t carry = (data & FLAG_CAR);
	processor_write(processor, addr, data);
	data = ((data >> 1) | ((processor->reg_p & FLAG_CAR) << 7));
	processor_write(processor, addr, data);
	set_nz(processor, data);
	set_flag(processor, FLAG_CAR, carry);
}

static void op_and(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor->reg_a = processor->reg_a & data;
	set_nz(processor, processor->reg_a);
}

static void op_ora(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor->reg_a = processor->reg_a | data;
	set_nz(processor, processor->reg_a);
}

static void op_eor(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	processor->reg_a = processor->reg_a ^ data;
	set_nz(processor, processor->reg_a);
}

static void op_bit(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	set_flag(processor, FLAG_ZER, (processor->reg_a & data) == 0);
	set_flag(processor, FLAG_NEG, data & FLAG_NEG);
	set_flag(processor, FLAG_OVF, data & FLAG_OVF);
}

static void op_cmp(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	set_flag(processor, FLAG_CAR, processor->reg_a >= data);
	set_flag(processor, FLAG_ZER, processor->reg_a == data);
	set_flag(processor, FLAG_NEG, (processor->reg_a - data) & FLAG_NEG);
}

static void op_cpx(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	set_flag(processor, FLAG_CAR, processor->reg_x >= data);
	set_flag(processor, FLAG_ZER, processor->reg_x == data);
	set_flag(processor, FLAG_NEG, (processor->reg_x - data) & FLAG_NEG);
}

static void op_cpy(processor_6502 * processor, bits_16 addr)
{
	uint8_t data = processor_read(processor, addr);
	set_flag(processor, FLAG_CAR, processor->reg_y >= data);
	set_flag(processor, FLAG_ZER, processor->reg_y == data);
	set_flag(processor, FLAG_NEG, (processor->reg_y - data) & FLAG_NEG);
}

static void op_bcc(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_CAR) == 0);
}

static void op_bcs(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, processor->reg_p & FLAG_CAR);
}

static void op_beq(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_ZER) > 0);
}

static void op_bne(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_ZER) == 0);
}
static void op_bpl(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_NEG) == 0);
}

static void op_bmi(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_NEG) > 0);
}

static void op_bvc(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_OVF) == 0);
}

static void op_bvs(processor_6502 * processor, bits_16 addr)
{
	handle_branch(processor, addr, (processor->reg_p & FLAG_OVF) > 0);
}

static void op_jmp(processor_6502 * processor, bits_16 addr)
{
	processor->reg_pc = addr;	
}

static void op_jsr(processor_6502 * processor, bits_16 addr)
{
	// Function containing the Addressing Mode Absolute
	// 	and op_jsr operations all together because
	//	of cycles reading and write order.
	bits_8	high_pc 	= (uint8_t)((processor->reg_pc & 0xFF00) >> 8);
	bits_8 	low_pc 		= (uint8_t)(processor->reg_pc & 0x00FF);
	bits_16 low_addr 	= processor_read(processor, addr);
	(void) stack_read(processor, processor->reg_sp);
	stack_push(processor, high_pc);
	stack_push(processor, low_pc);
	bits_16	high_addr	= processor_read(processor, (addr + 1) & 0xFFFF);
	bits_16 carry		= 0;
	processor->reg_pc	= little_endian_solver(low_addr, high_addr, 0, &carry);
}

static void op_rts(processor_6502 * processor, bits_16 addr)
{
	(void) stack_read(processor, processor->reg_sp);	// Dummy Read
	bits_8	low	= stack_pull(processor);
	bits_8	high	= stack_pull(processor);
	bits_16 carry	= 0;
	processor->reg_pc = little_endian_solver(low, high, 0, &carry);
	(void) processor_read(processor, processor->reg_pc);
	processor->reg_pc++;
	UNUSED(addr);
}

static void op_brk(processor_6502 * processor, bits_16 addr)
{
	handle_interrupt_irq_nmi(processor, INTERRUPT_IRQ_LW_ADDR, 1);
	UNUSED(addr);
}

static void op_rti(processor_6502 * processor, bits_16 addr)
{
	(void) stack_read(processor, processor->reg_sp);
	processor->reg_p	= stack_pull(processor);
	set_flag(processor, FLAG_BRK, 0);
	set_flag(processor, FLAG_NOEFFECT, 1);
	bits_16 low		= stack_pull(processor);
	bits_16 high		= stack_pull(processor);
	bits_16 carry		= 0;
	processor->reg_pc	= little_endian_solver(low, high, 0, &carry);
	UNUSED(addr);
}

static void op_pha(processor_6502 * processor, bits_16 addr)
{
	stack_push(processor, processor->reg_a);
	UNUSED(addr);
}

static void op_pla(processor_6502 * processor, bits_16 addr)
{
	(void) stack_read(processor, processor->reg_sp);
	processor->reg_a	= stack_pull(processor);
	set_nz(processor, processor->reg_a);
	UNUSED(addr);
}

static void op_php(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_BRK, 1); 			// Just for stacking
	stack_push(processor, processor->reg_p);
	set_flag(processor, FLAG_BRK, 0);			// Unset BREAK FLAG
	UNUSED(addr);
}

static void op_plp(processor_6502 * processor, bits_16 addr)
{
	(void) stack_read(processor, processor->reg_sp);
	processor->reg_p	= stack_pull(processor);
	set_flag(processor, FLAG_NOEFFECT, 1);			// Always SET
	set_flag(processor, FLAG_BRK, 0);
	UNUSED(addr);
}

static void op_clc(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_CAR, 0);
	UNUSED(addr);
}

static void op_sec(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_CAR, 1);
	UNUSED(addr);
}

static void op_cli(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_INT, 0);
	UNUSED(addr);
}

static void op_sei(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_INT, 1);
	UNUSED(addr);
}

static void op_cld(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_DEC, 0);
	UNUSED(addr);
}

static void op_sed(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_DEC, 1);
	UNUSED(addr);
}

static void op_clv(processor_6502 * processor, bits_16 addr)
{
	set_flag(processor, FLAG_OVF, 0);
	UNUSED(addr);
}

static void op_nop(processor_6502 * processor, bits_16 addr)
{
	// Literally NO OPERATION...
	UNUSED(processor);
	UNUSED(addr);
}

// >> PUBLIC FUNCTIONS <<


void execute_opcode(processor_6502 * processor)
{	
	bits_8 opcode = processor_read(processor, processor->reg_pc++);
	opcode_entry execute = op_table[opcode];
	
	// It's crashing when an UNDEFINED OPCODE is accessed
	if (execute.op == NULL) {
		printf("UNDEFINED OPCODE...\n");
		return;
	}
	printf("OPCODE: %s\n", execute.op_name);
	bits_16 address = execute.mode(processor, execute.force_oops);
	execute.op(processor, address);
}



uint8_t processor_6502_get_opcode_infos(uint8_t opcode, char **name, char **mode, uint8_t *mode_code)
{
	char *m_name = (char *) malloc( 256 );
	char *m_mode = (char *) malloc( 256 );
	uint8_t i = 0;
	uint8_t return_status = TRUE;

	if (op_table[opcode].op_name != NULL) {
		
		// Get OPCODE Name
		for (; op_table[opcode].op_name[i] != '\0'; i++ )
			m_name[i] = op_table[opcode].op_name[i];
		m_name[i] = '\0';
	
		i = 0;
	
		// Get ADDR MODE name
		for (; op_table[opcode].mode_name[i] != '\0'; i++ )
			m_mode[i] = op_table[opcode].mode_name[i];
		m_mode[i] = '\0';

		if (op_table[opcode].mode == mode_immediate)
			*mode_code = PROCESSOR_6502_ADDR_MODE_IMMEDIATE_CODE;
		else if (op_table[opcode].mode == mode_zero_page)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_CODE;
		else if (op_table[opcode].mode == mode_zero_page_x)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_X_CODE;
		else if (op_table[opcode].mode == mode_zero_page_y)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ZERO_PAGE_Y_CODE;
		else if (op_table[opcode].mode == mode_absolute)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ABSOLUTE_CODE;
		else if (op_table[opcode].mode == mode_absolute_x)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ABSOLUTE_X_CODE;
		else if (op_table[opcode].mode == mode_absolute_y)
			*mode_code = PROCESSOR_6502_ADDR_MODE_ABSOLUTE_Y_CODE;
		else if (op_table[opcode].mode == mode_indirect)
			*mode_code = PROCESSOR_6502_ADDR_MODE_INDIRECT_CODE;
		else if (op_table[opcode].mode == mode_indirect_x)
			*mode_code = PROCESSOR_6502_ADDR_MODE_INDIRECT_X_CODE;
		else if (op_table[opcode].mode == mode_indirect_y)
			*mode_code = PROCESSOR_6502_ADDR_MODE_INDIRECT_Y_CODE;
		else if (op_table[opcode].mode == mode_implied)
			*mode_code = PROCESSOR_6502_ADDR_MODE_IMPLIED_CODE;
		else if (op_table[opcode].mode == mode_relative)
			*mode_code = PROCESSOR_6502_ADDR_MODE_RELATIVE_CODE;
		else
			*mode_code = PROCESSOR_6502_ADDR_MODE_ERROR_CODE;
	} else {
		strcpy(m_name, "UNKOWN_OPCODE");
		strcpy(m_mode, "UNKOWN_ADDR_MODE");
		*mode_code = PROCESSOR_6502_ADDR_MODE_ERROR_CODE;
		return_status = FALSE;
	}

	*name = m_name;
	*mode = m_mode;
	return return_status;
}

void processor_6502_manual_initialization(processor_6502 * processor, uint16_t start_pc_addr)
{	
	processor->reg_a	= 0x00;
	processor->reg_x	= 0x00;
	processor->reg_y	= 0x00;
	processor->reg_sp	= 0x00;
	processor->reg_p	= FLAG_INITIAL_STATE;
	processor->read_write	= 0x00;
	processor->reg_pc	= start_pc_addr;

	processor->irq_pin	= IRQ_HIGH_VALUE;
	processor->nmi_pin	= NMI_HIGH_VALUE;
	processor->nmi_previous = NMI_HIGH_VALUE;
	processor->nmi_request	= 0;
	processor->cycles	= 0x00;

}

void processor_6502_initialization(processor_6502 * processor, uint8_t is_power_up)
{
	if (is_power_up) {
		processor->reg_a	= 0x00;
		processor->reg_x	= 0x00;
		processor->reg_y	= 0x00;
		processor->reg_sp	= 0x00;
		processor->reg_p	= 0x00;
		processor->read_write	= 0x00;

		processor->irq_pin	= IRQ_HIGH_VALUE;
		processor->nmi_pin	= NMI_HIGH_VALUE;
		processor->nmi_previous = NMI_HIGH_VALUE;
		processor->nmi_request	= 0;
		processor->cycles	= 0x00;
	}

	set_flag(processor, FLAG_NOEFFECT, 1);
	set_flag(processor, FLAG_INT, 1);	
	(void) processor_read(processor, processor->reg_pc & 0xFFFF);				// Cycle 1
	(void) processor_read(processor, (processor->reg_pc + 1) & 0xFFFF);			// Cycle 2
	(void) stack_read(processor, (processor->reg_sp--) & 0xFF);				// Cycle 3
	(void) stack_read(processor, (processor->reg_sp--) & 0xFF);				// Cycle 4
	(void) stack_read(processor, (processor->reg_sp--) & 0xFF);				// Cycle 5
	uint16_t low_pc = processor_read(processor, INTERRUPT_RESET_LW_ADDR);			// Cycle 6
	uint16_t high_pc = processor_read(processor, (INTERRUPT_RESET_LW_ADDR + 1) & 0xFFFF);	// Cycle 7
	uint16_t carry = 0;
	processor->reg_pc = little_endian_solver(low_pc, high_pc, 0, &carry);
}

void processor_6502_execute_opcode(processor_6502 * processor)
{
	execute_opcode(processor);
	response_interrupt_request_if_any(processor);
}


