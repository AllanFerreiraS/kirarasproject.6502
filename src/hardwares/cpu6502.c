/*
 * cpu6502.c - NMOS 6502 CPU core
 *
 * Executes the 151 documented opcodes. Illegal opcodes are not implemented yet.
 * Cycles are counted at every bus read and every bus write.
 * Every opcode simulates all of the necessary cycles to execute the opcode, even
 * dummy reads, dummy writes and page wrappers are simulated.
 *
 * Interrupts are also simulated. The cpu6502 detects an IRQ request when the IRQ pin has low value,
 * and the NMI when there is a change of high to low value in the NMI pin. The difference between
 * IRQ and NMI is that the cpu6502 ignores the request if the Interrupt Disable flag is activated.
 * The NMI is handled at the momment that it is requested. So, NMI has a higher priority
 * compared to IRQ.
 *
 * A bug in the Indirect Addressing Mode is also simulated. When a page wrapper is required, instead
 * of changing to the next page, the page remains the same.
 *
 * How to include:
 * Take a look at the cpu6502.h and in the examples file.
 *
 * References:	https://www.nesdev.org/wiki/CPU			(A lot of information about the 6502 processor)
 *		https://github.com/SingleStepTests/65x02	(Helped me implementing accurate cycles)
 */

#include "./../../include/hardwares/cpu6502.h"
#include <stdio.h>
#include <string.h>

/*
 * LATER CHECKS:
 * -> processor_6502_execute_opcode has to handle UNKOWN OPCODES
 * -> Decimal. Affects the D Flag: CLD, PLP, RTI, and SED. (BRK)
 */

// Function Pointers to handle Addressing Modes and Opcodes.
typedef u16  (*addr_fn) (struct cpu6502 *cpu, u8 force_oops);
typedef void (*op_fn) (struct cpu6502 *cpu, u16 addr);

struct opcode_entry {
	const char	*op_name;
	const char	*mode_name;
	op_fn		op;
	addr_fn		mode;
	u8		force_oops; // Force Dummy Cycles;
};

// Bus
static u8   cpu6502_read(struct cpu6502 *cpu, u16 address);
static void cpu6502_write(struct cpu6502 *cpu, u16 address, u8 data);

// Addressing Mode Functions
static u16 mode_immediate(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_zero_page(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_zero_page_x(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_zero_page_y(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_absolute(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_absolute_x(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_absolute_y(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_indirect(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_indirect_x(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_indirect_y(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_implied(struct cpu6502 *cpu, u8 force_oops);
static u16 mode_relative(struct cpu6502 *cpu, u8 force_oops);

// OPCODE Functions
// Access:
static void op_lda(struct cpu6502 *cpu, u16 addr);
static void op_ldx(struct cpu6502 *cpu, u16 addr);
static void op_ldy(struct cpu6502 *cpu, u16 addr);
static void op_sta(struct cpu6502 *cpu, u16 addr);
static void op_stx(struct cpu6502 *cpu, u16 addr);
static void op_sty(struct cpu6502 *cpu, u16 addr);
// Transfer:
static void op_tax(struct cpu6502 *cpu, u16 addr);
static void op_tay(struct cpu6502 *cpu, u16 addr);
static void op_txa(struct cpu6502 *cpu, u16 addr);
static void op_tya(struct cpu6502 *cpu, u16 addr);
// Arithmetic:
static void op_adc(struct cpu6502 *cpu, u16 addr);
static void op_sbc(struct cpu6502 *cpu, u16 addr);
static void op_inc(struct cpu6502 *cpu, u16 addr);
static void op_dec(struct cpu6502 *cpu, u16 addr);
static void op_inx(struct cpu6502 *cpu, u16 addr);
static void op_dex(struct cpu6502 *cpu, u16 addr);
static void op_iny(struct cpu6502 *cpu, u16 addr);
static void op_dey(struct cpu6502 *cpu, u16 addr);
// Shift:
static void op_asl(struct cpu6502 *cpu, u16 addr);
static void op_lsr(struct cpu6502 *cpu, u16 addr);
static void op_rol(struct cpu6502 *cpu, u16 addr);
static void op_ror(struct cpu6502 *cpu, u16 addr);
static void op_asl_accum(struct cpu6502 *cpu, u16 addr);
static void op_lsr_accum(struct cpu6502 *cpu, u16 addr);
static void op_rol_accum(struct cpu6502 *cpu, u16 addr);
static void op_ror_accum(struct cpu6502 *cpu, u16 addr);
// Bitwise:
static void op_and(struct cpu6502 *cpu, u16 addr);
static void op_ora(struct cpu6502 *cpu, u16 addr);
static void op_eor(struct cpu6502 *cpu, u16 addr);
static void op_bit(struct cpu6502 *cpu, u16 addr);
// Compare:
static void op_cmp(struct cpu6502 *cpu, u16 addr);
static void op_cpx(struct cpu6502 *cpu, u16 addr);
static void op_cpy(struct cpu6502 *cpu, u16 addr);
// Branch:
static void op_bcc(struct cpu6502 *cpu, u16 addr);
static void op_bcs(struct cpu6502 *cpu, u16 addr);
static void op_beq(struct cpu6502 *cpu, u16 addr);
static void op_bne(struct cpu6502 *cpu, u16 addr);
static void op_bpl(struct cpu6502 *cpu, u16 addr);
static void op_bmi(struct cpu6502 *cpu, u16 addr);
static void op_bvc(struct cpu6502 *cpu, u16 addr);
static void op_bvs(struct cpu6502 *cpu, u16 addr);
// Jump:
static void op_jmp(struct cpu6502 *cpu, u16 addr);
static void op_jsr(struct cpu6502 *cpu, u16 addr);
static void op_rts(struct cpu6502 *cpu, u16 addr);
static void op_brk(struct cpu6502 *cpu, u16 addr);
static void op_rti(struct cpu6502 *cpu, u16 addr);
// Stack
static void op_pha(struct cpu6502 *cpu, u16 addr);
static void op_pla(struct cpu6502 *cpu, u16 addr);
static void op_php(struct cpu6502 *cpu, u16 addr);
static void op_plp(struct cpu6502 *cpu, u16 addr);
static void op_tsx(struct cpu6502 *cpu, u16 addr);
static void op_txs(struct cpu6502 *cpu, u16 addr);
// Flags
static void op_clc(struct cpu6502 *cpu, u16 addr);
static void op_sec(struct cpu6502 *cpu, u16 addr);
static void op_cli(struct cpu6502 *cpu, u16 addr);
static void op_sei(struct cpu6502 *cpu, u16 addr);
static void op_cld(struct cpu6502 *cpu, u16 addr);
static void op_sed(struct cpu6502 *cpu, u16 addr);
static void op_clv(struct cpu6502 *cpu, u16 addr);
// Other
static void op_nop(struct cpu6502 *cpu, u16 addr);


// Flags Management
static void set_flag(struct cpu6502 *cpu, u8 flag_mask, int condition);
static void set_nz(struct cpu6502 *cpu, u8 data);

// Stack Management
static void stack_push(struct cpu6502 *cpu, u8 data);
static u8   stack_pull(struct cpu6502 *cpu);
static u8   stack_read(struct cpu6502 *cpu, u8 stack);

// Others
static u16  little_endian_solver(u16 low, u16 high, u8 add, u16 *carry);
static u8   oops(u8 force_oops, u8 condition);
static void nmi_request_update(struct cpu6502 *cpu);

// Handles (Helpers)
static void handle_interrupt_irq_nmi(struct cpu6502 *cpu, u16 interrupt_lw_addr, u8 brk_triggered);
static void handle_op_adc_decimal_mode(struct cpu6502 *cpu, u8 data);
static void handle_op_adc_binary_mode(struct cpu6502 *cpu, u8 data);
static void handle_op_sbc_decimal_mode(struct cpu6502 *cpu, u8 data);
static void handle_op_sbc_binary_mode(struct cpu6502 *cpu, u8 data);
static void handle_branch(struct cpu6502 *cpu, u16 addr, u8 condition);

static const struct opcode_entry op_table[CPU6502_OPCODES_AMOUNT] ={
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

static u16 little_endian_solver(u16 low, u16 high, u8 add, u16 *carry)
{
	*carry = ((low + add) & 0x100);
	return ( (high << 8) | ((low + add) & 0xFF) );
}

static u8 oops(u8 force_oops, u8 condition)
{
	return (u8)(force_oops || condition);
}

static void nmi_request_update(struct cpu6502 *cpu)
{
	if ( (cpu->nmi_previous == CPU6502_NMI_HIGH) && (cpu->nmi_pin == CPU6502_NMI_LOW) )
		cpu->nmi_request = true;

	cpu->nmi_previous = cpu->nmi_pin;
}





// >> BUS <<

static u8 cpu6502_read(struct cpu6502 *cpu, u16 address)
{
	cpu->cycles++;
	cpu->read_write = CPU6502_FLAG_READ;
	u8 data = cpu->bus.read_bus((void *) cpu->bus.emulator, address);
	//nmi_request_update(cpu);
	return data;
}
static void cpu6502_write(struct cpu6502 *cpu, u16 address, u8 data)
{
	cpu->cycles++;
	cpu->read_write = CPU6502_FLAG_WRITE;
	cpu->bus.write_bus((void *) cpu->bus.emulator, address, data);
	nmi_request_update(cpu);
}






// >> FLAGS <<

static void set_flag(struct cpu6502 *cpu, u8 flag_mask, int condition)
{
	if (condition)
		cpu->reg_p |= flag_mask;
	else
		cpu->reg_p &= (~flag_mask);
}

static void set_nz(struct cpu6502 *cpu, u8 data)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, (data == 0));
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, ((data & CPU6502_STATUS_FLAG_NEG) > 0));
}






// >> STACKS <<

static void stack_push(struct cpu6502 *cpu, u8 data)
{
	cpu6502_write(cpu, CPU6502_STACK_LOWER_ADDR + cpu->reg_sp, data);
	cpu->reg_sp = (cpu->reg_sp - 1) & 0xFF;
}

static u8 stack_pull(struct cpu6502 *cpu)
{
	cpu->reg_sp = (cpu->reg_sp + 1) & 0xFF;
	return cpu6502_read(cpu, CPU6502_STACK_LOWER_ADDR + cpu->reg_sp);
}

static u8 stack_read(struct cpu6502 *cpu, u8 stack)
{
	return cpu6502_read(cpu, CPU6502_STACK_LOWER_ADDR + stack);
}






// >> ADDRESSING MODES <<

static u16 mode_immediate(struct cpu6502 *cpu, u8 force_oops)
{
	u16 addr = cpu->reg_pc++;
	UNUSED(force_oops);
	return addr;
}

static u16 mode_zero_page(struct cpu6502 *cpu, u8 force_oops)
{
	u16 addr = cpu6502_read(cpu, cpu->reg_pc++);
	UNUSED(force_oops);
	return addr;
}

static u16 mode_zero_page_x(struct cpu6502 *cpu, u8 force_oops)
{
	u16 addr = cpu6502_read(cpu, cpu->reg_pc++);
	(void) cpu6502_read(cpu, addr);				// Dummy Read
	addr = (addr + cpu->reg_x) & 0xFF;
	UNUSED(force_oops);
	return addr;
}

static u16 mode_zero_page_y(struct cpu6502 *cpu, u8 force_oops)
{
	u16 addr = cpu6502_read(cpu, cpu->reg_pc++);
	(void) cpu6502_read(cpu, addr); 			// Dummy Read
	addr = (addr + cpu->reg_y) & 0xFF;
	UNUSED(force_oops);
	return addr;
}

static u16 mode_absolute(struct cpu6502 *cpu, u8 force_oops)
{
	u16 low		= cpu6502_read(cpu, cpu->reg_pc++);
	u16 high	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 carry 	= 0;					// Ignore Carry in this case
	u16 addr 	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
	return addr;
}

static u16 mode_absolute_x(struct cpu6502 *cpu, u8 force_oops)
{
	u16 low 	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 high 	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 carry 	= 0;
	u16 addr 	= little_endian_solver(low, high, cpu->reg_x, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) cpu6502_read(cpu, addr); 		// Dummy Read
		addr 	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static u16 mode_absolute_y(struct cpu6502 *cpu, u8 force_oops)
{
	u16 low 	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 high 	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 carry	= 0;
	u16 addr	= little_endian_solver(low, high, cpu->reg_y, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) cpu6502_read(cpu, addr);			// Dummy Read
		addr	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static u16 mode_indirect(struct cpu6502 *cpu, u8 force_oops)
{
	u16 p_low	= cpu6502_read(cpu, cpu->reg_pc++) & 0xFF;
	u16 p_high	= cpu6502_read(cpu, cpu->reg_pc++) & 0xFF;
	u16 carry	= 0;
	u16 low_addr	= little_endian_solver(p_low, p_high, 0, &carry);
	u16 low		= cpu6502_read(cpu, low_addr);
	u16 high 	= 0;
	// CPU BUG
	if ( ((low_addr & 0x00FF) + 1) > 0x00FF )
		high	= cpu6502_read(cpu, low_addr & 0xFF00);
	else
		high	= cpu6502_read(cpu, (low_addr + 1) & 0xFFFF);
	u16 addr	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
	return addr;
}

static u16 mode_indirect_x(struct cpu6502 *cpu, u8 force_oops)
{
	u16 l_addr	= cpu6502_read(cpu, cpu->reg_pc++);
	(void) cpu6502_read(cpu, l_addr);			// Dummy Read
	l_addr 		= (l_addr + cpu->reg_x) & 0xFF;
	u16 low	= cpu6502_read(cpu, l_addr);
	u16 high	= cpu6502_read(cpu, (l_addr + 1) & 0xFF);
	u16 carry	= 0;					// Ignore Carry in this case.
	u16 addr 	= little_endian_solver(low, high, 0, &carry);
	UNUSED(force_oops);
	return addr;
}

static u16 mode_indirect_y(struct cpu6502 *cpu, u8 force_oops)
{
	u16 l_addr 	= cpu6502_read(cpu, cpu->reg_pc++);
	u16 low 	= cpu6502_read(cpu, l_addr);
	u16 high 	= cpu6502_read(cpu, (l_addr + 1) & 0xFF);
	u16 carry	= 0;
	u16 addr 	= little_endian_solver(low, high, cpu->reg_y, &carry);
	if ( oops(force_oops, (carry & 0x100) > 0) ) {
		(void) cpu6502_read(cpu, addr);			// Dummy Read
		addr	= (addr + carry) & 0xFFFF;
	}
	return addr;
}

static u16 mode_implied(struct cpu6502 *cpu, u8 force_oops)
{
	(void) cpu6502_read(cpu, cpu->reg_pc);
	UNUSED(force_oops);
	return cpu->reg_pc;
}

static u16 mode_relative(struct cpu6502 *cpu, u8 force_oops)
{
	u16 addr = cpu->reg_pc++;
	UNUSED(force_oops);
	return addr;
}






// >> HANDLE FUNCTIONS <<

static void handle_op_adc_decimal_mode(struct cpu6502 *cpu, u8 data)
{
	// For more info: https://6502.org/tutorials/decimal_mode.html#A
	u16 r		= (cpu->reg_a & 0x0F) + (data & 0X0F) + (cpu->reg_p & CPU6502_STATUS_FLAG_CAR);
	u16 r_bin	= (cpu->reg_a) + (data) + (cpu->reg_p & CPU6502_STATUS_FLAG_CAR);
	u16 old_a 	= cpu->reg_a;

	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, ((r_bin & 0xFF) == 0x00));

	if (r >= 0x0A)
		r = ((r + 0x06) & 0x0F) + 0x10;

	r = (cpu->reg_a & 0xF0) + (data & 0xF0) + r;

	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (r & CPU6502_STATUS_FLAG_NEG));
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, (r ^ old_a) & (r ^ ((u16)data)) & 0x0080);

	if (r >= 0xA0)
		r = r + 0x60;

	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, (r >= 0x100));
	cpu->reg_a = r & 0xFF;
}

static void handle_op_sbc_decimal_mode(struct cpu6502 *cpu, u8 data)
{
	// For more info: https://6502.org/tutorials/decimal_mode.html#A
	u16 r		= (cpu->reg_a & 0x0F) - (data & 0X0F) + (cpu->reg_p & CPU6502_STATUS_FLAG_CAR) - 1;
	u16 r_bin	= (cpu->reg_a) - (data) + (cpu->reg_p & CPU6502_STATUS_FLAG_CAR) - 1;
	u16 old_a 	= cpu->reg_a;

	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (r_bin & CPU6502_STATUS_FLAG_NEG));
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, (~((i16)r_bin)) < 0x00);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, ((r_bin & 0xFF) == 0x00));
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, (r_bin ^ old_a) & (r_bin ^ (~(u16)data)) & 0x0080);

	if ((i16)r < 0)
		r = ((r - 0x06) & 0x0F) - 0x10;

	r = (cpu->reg_a & 0xF0) - (data & 0xF0) + r;
	if ((i16)r < 0)
		r = (r - 0x60) & 0xFF;

	cpu->reg_a = r & 0xFF;
}

static void handle_op_adc_binary_mode(struct cpu6502 *cpu, u8 data)
{
	u16 r		= ((u16)cpu->reg_a) + ((u16)data) + ((u16)(cpu->reg_p & CPU6502_STATUS_FLAG_CAR));
	u16 old_a 	= cpu->reg_a;
	cpu->reg_a 	= r & 0xFF;

	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, (r > 0xFF));
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, (cpu->reg_a & 0xFF) == 0);
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, (r ^ old_a) & (r ^ (u16)data) & 0x0080);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (cpu->reg_a & CPU6502_STATUS_FLAG_NEG) > 0);
}

static void handle_op_sbc_binary_mode(struct cpu6502 *cpu, u8 data)
{
	u16 r		= ((u16)cpu->reg_a) + ~((u16)data) + ((u16)(cpu->reg_p & CPU6502_STATUS_FLAG_CAR));
	u16 old_a 	= cpu->reg_a;
	cpu->reg_a	= r & 0xFF;

	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, (~((i16)r)) < 0x00);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, (cpu->reg_a & 0xFF) == 0);
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, (r ^ old_a) & (r ^ (~(u16)data)) & 0x0080);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (cpu->reg_a & CPU6502_STATUS_FLAG_NEG) > 0);
}

static void handle_interrupt_irq_nmi(struct cpu6502 *cpu, u16 interrupt_lw_addr, u8 brk_triggered)
{
	if (brk_triggered && (interrupt_lw_addr == CPU6502_INTERRUPT_IRQ_LW_ADDR)) {
		(void) cpu6502_read(cpu, ((u16)(cpu->reg_pc - 1) & 0xFFFF));	//Force another read
		set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 1);			// Just for the stack
	} else {
		(void) cpu6502_read(cpu, cpu->reg_pc);				// Force an OPCODE read
		(void) cpu6502_read(cpu, cpu->reg_pc + 1);			// Force another read
		set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 0);
	}

	u8 high_pc = (u8)((cpu->reg_pc & 0xFF00) >> 8);
	u8 low_pc = (u8)(cpu->reg_pc & 0x00FF);
	stack_push(cpu, high_pc);
	stack_push(cpu, low_pc);
	stack_push(cpu, cpu->reg_p);
	set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 0);				// Make break flag unset
	set_flag(cpu, CPU6502_STATUS_FLAG_INT, 1);				// Set Interrupt Disabled
	u16 low_addr	= cpu6502_read(cpu, interrupt_lw_addr);
	u16 high_addr	= cpu6502_read(cpu, (interrupt_lw_addr + 1) & 0xFFFF);
	u16 carry	= 0;
	cpu->reg_pc	= little_endian_solver(low_addr, high_addr, 0, &carry);
}

static void handle_branch(struct cpu6502 *cpu, u16 addr, u8 condition)
{
	i8 relat	= cpu6502_read(cpu, addr);
	u16 low 	= cpu->reg_pc & 0x00FF;
	u16 high 	= cpu->reg_pc & 0xFF00;
	u16 low_buf= low;
	if (condition) {
		(void) cpu6502_read(cpu, cpu->reg_pc);
		low = (low + (i8)relat) & 0x00FF;
		cpu->reg_pc = high | low;
		if ((low_buf + (i8)relat) > 0x00FF) {
			cpu6502_read(cpu, cpu->reg_pc);
			cpu->reg_pc += 0x100;
		}
		else if ((low_buf + (i8)relat) < 0) {
			cpu6502_read(cpu, cpu->reg_pc);
			cpu->reg_pc -= 0x100;
		}
	}
}







// >> OPCODES <<

static void op_lda(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_a = cpu6502_read(cpu, addr);
	set_nz(cpu, cpu->reg_a);
}

static void op_ldx(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_x = cpu6502_read(cpu, addr);
	set_nz(cpu, cpu->reg_x);
}

static void op_ldy(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_y = cpu6502_read(cpu, addr);
	set_nz(cpu, cpu->reg_y);
}

static void op_sta(struct cpu6502 *cpu, u16 addr)
{
	cpu6502_write(cpu, addr, cpu->reg_a);
}

static void op_stx(struct cpu6502 *cpu, u16 addr)
{
	cpu6502_write(cpu, addr, cpu->reg_x);
}

static void op_sty(struct cpu6502 *cpu, u16 addr)
{
	cpu6502_write(cpu, addr, cpu->reg_y);
}

static void op_tax(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_x = cpu->reg_a;
	set_nz(cpu, cpu->reg_x);
	UNUSED(addr);
}

static void op_tay(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_y = cpu->reg_a;
	set_nz(cpu, cpu->reg_y);
	UNUSED(addr);
}

static void op_tsx(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_x = cpu->reg_sp;
	set_nz(cpu, cpu->reg_x);
	UNUSED(addr);
}

static void op_txa(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_a = cpu->reg_x;
	set_nz(cpu, cpu->reg_a);
	UNUSED(addr);
}

static void op_txs(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_sp = cpu->reg_x;
	UNUSED(addr);
}

static void op_tya(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_a = cpu->reg_y;
	set_nz(cpu, cpu->reg_a);
	UNUSED(addr);
}

static void op_adc(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	if (cpu->reg_p & CPU6502_STATUS_FLAG_DEC)
		handle_op_adc_decimal_mode(cpu, data);
	else
		handle_op_adc_binary_mode(cpu, data);
}

static void op_sbc(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	if (cpu->reg_p & CPU6502_STATUS_FLAG_DEC)
		handle_op_sbc_decimal_mode(cpu, data);
	else
		handle_op_sbc_binary_mode(cpu, data);
}

static void op_inc(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu6502_write(cpu, addr, data);
	data++;
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
}

static void op_dec(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu6502_write(cpu, addr, data);
	data--;
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
}

static void op_inx(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_x++;
	set_nz(cpu, cpu->reg_x);
	UNUSED(addr);
}

static void op_dex(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_x--;
	set_nz(cpu, cpu->reg_x);
	UNUSED(addr);
}

static void op_iny(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_y++;
	set_nz(cpu, cpu->reg_y);
	UNUSED(addr);
}

static void op_dey(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_y--;
	set_nz(cpu, cpu->reg_y);
	UNUSED(addr);
}

static void op_asl_accum(struct cpu6502 *cpu, u16 addr)
{
	u8 carry = ((cpu->reg_a & 0b10000000) >> 7); // Get the last bit
	cpu->reg_a = cpu->reg_a << 1;
	set_nz(cpu, cpu->reg_a);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_asl(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	u8 carry = ((data & 0b10000000) >> 7); // Get the last bit
	cpu6502_write(cpu, addr, data);
	data = data << 1;
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
}

static void op_lsr_accum(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, cpu->reg_a & CPU6502_STATUS_FLAG_CAR);
	cpu->reg_a = cpu->reg_a >> 1;
	set_nz(cpu, cpu->reg_a);
	UNUSED(addr);
}

static void op_lsr(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu6502_write(cpu, addr, data);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, data & CPU6502_STATUS_FLAG_CAR);
	data = data >> 1;
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
}

static void op_rol_accum(struct cpu6502 *cpu, u16 addr)
{
	u8 carry = ((cpu->reg_a & 0b10000000) >> 7); // Get the last bit
	cpu->reg_a = ((cpu->reg_a << 1) | (cpu->reg_p & CPU6502_STATUS_FLAG_CAR));
	set_nz(cpu, cpu->reg_a);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_rol(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
 	u8 carry = ((data & 0b10000000) >> 7); // Get the last bit
	cpu6502_write(cpu, addr, data);
	data = ((data << 1) | (cpu->reg_p & CPU6502_STATUS_FLAG_CAR));
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
}

static void op_ror_accum(struct cpu6502 *cpu, u16 addr)
{
	u8 carry = ((cpu->reg_a & CPU6502_STATUS_FLAG_CAR));
	cpu->reg_a = ((cpu->reg_a >> 1) | ((cpu->reg_p & CPU6502_STATUS_FLAG_CAR) << 7));
	set_nz(cpu, cpu->reg_a);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
	UNUSED(addr);
}

static void op_ror(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	u8 carry = (data & CPU6502_STATUS_FLAG_CAR);
	cpu6502_write(cpu, addr, data);
	data = ((data >> 1) | ((cpu->reg_p & CPU6502_STATUS_FLAG_CAR) << 7));
	cpu6502_write(cpu, addr, data);
	set_nz(cpu, data);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, carry);
}

static void op_and(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu->reg_a = cpu->reg_a & data;
	set_nz(cpu, cpu->reg_a);
}

static void op_ora(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu->reg_a = cpu->reg_a | data;
	set_nz(cpu, cpu->reg_a);
}

static void op_eor(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	cpu->reg_a = cpu->reg_a ^ data;
	set_nz(cpu, cpu->reg_a);
}

static void op_bit(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, (cpu->reg_a & data) == 0);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, data & CPU6502_STATUS_FLAG_NEG);
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, data & CPU6502_STATUS_FLAG_OVF);
}

static void op_cmp(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, cpu->reg_a >= data);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, cpu->reg_a == data);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (cpu->reg_a - data) & CPU6502_STATUS_FLAG_NEG);
}

static void op_cpx(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, cpu->reg_x >= data);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, cpu->reg_x == data);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (cpu->reg_x - data) & CPU6502_STATUS_FLAG_NEG);
}

static void op_cpy(struct cpu6502 *cpu, u16 addr)
{
	u8 data = cpu6502_read(cpu, addr);
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, cpu->reg_y >= data);
	set_flag(cpu, CPU6502_STATUS_FLAG_ZER, cpu->reg_y == data);
	set_flag(cpu, CPU6502_STATUS_FLAG_NEG, (cpu->reg_y - data) & CPU6502_STATUS_FLAG_NEG);
}

static void op_bcc(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_CAR) == 0);
}

static void op_bcs(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, cpu->reg_p & CPU6502_STATUS_FLAG_CAR);
}

static void op_beq(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_ZER) > 0);
}

static void op_bne(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_ZER) == 0);
}
static void op_bpl(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_NEG) == 0);
}

static void op_bmi(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_NEG) > 0);
}

static void op_bvc(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_OVF) == 0);
}

static void op_bvs(struct cpu6502 *cpu, u16 addr)
{
	handle_branch(cpu, addr, (cpu->reg_p & CPU6502_STATUS_FLAG_OVF) > 0);
}

static void op_jmp(struct cpu6502 *cpu, u16 addr)
{
	cpu->reg_pc = addr;
}

static void op_jsr(struct cpu6502 *cpu, u16 addr)
{
	// Function containing the Addressing Mode Absolute
	// 	and op_jsr operations all together because
	//	of cycles reading and write order.
	u8 high_pc	= (u8)((cpu->reg_pc & 0xFF00) >> 8);
	u8 low_pc 	= (u8)(cpu->reg_pc & 0x00FF);
	u16 low_addr 	= cpu6502_read(cpu, addr);
	(void) stack_read(cpu, cpu->reg_sp);
	stack_push(cpu, high_pc);
	stack_push(cpu, low_pc);
	u16 high_addr	= cpu6502_read(cpu, (addr + 1) & 0xFFFF);
	u16 carry	= 0;
	cpu->reg_pc	= little_endian_solver(low_addr, high_addr, 0, &carry);
}

static void op_rts(struct cpu6502 *cpu, u16 addr)
{
	(void) stack_read(cpu, cpu->reg_sp);	// Dummy Read
	u8 low	  = stack_pull(cpu);
	u8 high	  = stack_pull(cpu);
	u16 carry = 0;
	cpu->reg_pc = little_endian_solver(low, high, 0, &carry);
	(void) cpu6502_read(cpu, cpu->reg_pc);
	cpu->reg_pc++;
	UNUSED(addr);
}

static void op_brk(struct cpu6502 *cpu, u16 addr)
{
	handle_interrupt_irq_nmi(cpu, CPU6502_INTERRUPT_IRQ_LW_ADDR, 1);
	UNUSED(addr);
}

static void op_rti(struct cpu6502 *cpu, u16 addr)
{
	(void) stack_read(cpu, cpu->reg_sp);
	cpu->reg_p	= stack_pull(cpu);
	set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 0);
	set_flag(cpu, CPU6502_STATUS_FLAG_NOEFFECT, 1);
	u16 low		= stack_pull(cpu);
	u16 high	= stack_pull(cpu);
	u16 carry	= 0;
	cpu->reg_pc	= little_endian_solver(low, high, 0, &carry);
	UNUSED(addr);
}

static void op_pha(struct cpu6502 *cpu, u16 addr)
{
	stack_push(cpu, cpu->reg_a);
	UNUSED(addr);
}

static void op_pla(struct cpu6502 *cpu, u16 addr)
{
	(void) stack_read(cpu, cpu->reg_sp);
	cpu->reg_a = stack_pull(cpu);
	set_nz(cpu, cpu->reg_a);
	UNUSED(addr);
}

static void op_php(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 1); // Just for stacking
	stack_push(cpu, cpu->reg_p);
	set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 0); // Unset BREAK FLAG
	UNUSED(addr);
}

static void op_plp(struct cpu6502 *cpu, u16 addr)
{
	(void) stack_read(cpu, cpu->reg_sp);
	cpu->reg_p = stack_pull(cpu);
	set_flag(cpu, CPU6502_STATUS_FLAG_NOEFFECT, 1);	// Always SET
	set_flag(cpu, CPU6502_STATUS_FLAG_BRK, 0);
	UNUSED(addr);
}

static void op_clc(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, 0);
	UNUSED(addr);
}

static void op_sec(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_CAR, 1);
	UNUSED(addr);
}

static void op_cli(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_INT, 0);
	UNUSED(addr);
}

static void op_sei(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_INT, 1);
	UNUSED(addr);
}

static void op_cld(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_DEC, 0);
	UNUSED(addr);
}

static void op_sed(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_DEC, 1);
	UNUSED(addr);
}

static void op_clv(struct cpu6502 *cpu, u16 addr)
{
	set_flag(cpu, CPU6502_STATUS_FLAG_OVF, 0);
	UNUSED(addr);
}

static void op_nop(struct cpu6502 *cpu, u16 addr)
{
	// Literally NO OPERATION...
	UNUSED(cpu);
	UNUSED(addr);
}





// >> PUBLIC FUNCTIONS <<

enum cpu6502_errno cpu6502_bus_prepare(struct cpu6502 *cpu, u8 (*read)(void *emulator, u16 addr), void (*write)(void *emulator, u16 addr, u8 data), void *emulator)
{
	if (read == NULL || write == NULL)
		return CPU6502_ERRNO_BUS_FUNCTION_NULL;
	cpu->bus = (struct buses){ read, write, emulator, 0x00 };
	return CPU6502_ERRNO_OK;
}

void cpu6502_init(struct cpu6502 *cpu, u8 power_up)
{
	if (power_up) {
		cpu->reg_a	= 0x00;
		cpu->reg_x	= 0x00;
		cpu->reg_y	= 0x00;
		cpu->reg_sp	= 0x00;
		cpu->reg_p	= 0x00;
		cpu->read_write	= 0x00;

		cpu->irq_pin	= CPU6502_IRQ_HIGH;
		cpu->nmi_pin	= CPU6502_NMI_HIGH;
		cpu->nmi_previous = CPU6502_NMI_HIGH;
		cpu->nmi_request = false;
		cpu->cycles	= 0x00;
	}

	set_flag(cpu, CPU6502_STATUS_FLAG_NOEFFECT, 1);
	set_flag(cpu, CPU6502_STATUS_FLAG_INT, 1);
	(void) cpu6502_read(cpu, cpu->reg_pc & 0xFFFF); // Cycle 1
	(void) cpu6502_read(cpu, (cpu->reg_pc + 1) & 0xFFFF); // Cycle 2
	(void) stack_read(cpu, (cpu->reg_sp--) & 0xFF);	// Cycle 3
	(void) stack_read(cpu, (cpu->reg_sp--) & 0xFF);	// Cycle 4
	(void) stack_read(cpu, (cpu->reg_sp--) & 0xFF);	// Cycle 5
	u16 low_pc	= cpu6502_read(cpu, CPU6502_INTERRUPT_RESET_LW_ADDR); // Cycle 6
	u16 high_pc	= cpu6502_read(cpu, (CPU6502_INTERRUPT_RESET_LW_ADDR + 1) & 0xFFFF); // Cycle 7
	u16 carry	= 0;
	cpu->reg_pc	= little_endian_solver(low_pc, high_pc, 0, &carry);
}

void cpu6502_init_manual(struct cpu6502 *cpu, u16 reg_pc)
{
	cpu->reg_a	= 0x00;
	cpu->reg_x	= 0x00;
	cpu->reg_y	= 0x00;
	cpu->reg_sp	= 0x00;
	cpu->reg_p	= CPU6502_STATUS_FLAG_INITIAL_STATE;
	cpu->read_write	= 0x00;
	cpu->reg_pc	= reg_pc;

	cpu->irq_pin	= CPU6502_IRQ_HIGH;
	cpu->nmi_pin	= CPU6502_NMI_HIGH;
	cpu->nmi_previous = CPU6502_NMI_HIGH;
	cpu->nmi_request = false;
	cpu->cycles = 0x00;
}

enum cpu6502_errno cpu6502_opcode_execute(struct cpu6502 *cpu)
{
	u8 opcode = cpu6502_read(cpu, cpu->reg_pc++);
	struct opcode_entry execute = op_table[opcode];

	// It's crashing when an UNDEFINED OPCODE is accessed
	if (execute.op == NULL)
		return CPU6502_ERRNO_ILLEGAL_OPCODE;
	u16 address = execute.mode(cpu, execute.force_oops);
	execute.op(cpu, address);
	return CPU6502_ERRNO_OK;
}

void cpu6502_interrupt_handle_request_if_any(struct cpu6502 *cpu)
{
	if (cpu->nmi_request) {
		//printf("NMI\n");
		handle_interrupt_irq_nmi(cpu, CPU6502_INTERRUPT_NMI_LW_ADDR, 0);
		cpu->nmi_request = false;
	}

	if ( (cpu->irq_pin == CPU6502_IRQ_LOW) && ((cpu->reg_p & CPU6502_STATUS_FLAG_INT) == 0) ) {
		//printf("IRQ\n");
		handle_interrupt_irq_nmi(cpu, CPU6502_INTERRUPT_IRQ_LW_ADDR, 0);
		cpu->irq_pin = CPU6502_IRQ_HIGH;
	}
}

u8 cpu6502_opcode_get_infos(u8 opcode, char **name, char **mode, u8 *mode_code)
{
	char *m_name = (char *) malloc( 256 );
	char *m_mode = (char *) malloc( 256 );
	u8 i = 0;
	u8 return_status = true;

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
			*mode_code = CPU6502_ADDR_MODE_IMMEDIATE_CODE;
		else if (op_table[opcode].mode == mode_zero_page)
			*mode_code = CPU6502_ADDR_MODE_ZERO_PAGE_CODE;
		else if (op_table[opcode].mode == mode_zero_page_x)
			*mode_code = CPU6502_ADDR_MODE_ZERO_PAGE_X_CODE;
		else if (op_table[opcode].mode == mode_zero_page_y)
			*mode_code = CPU6502_ADDR_MODE_ZERO_PAGE_Y_CODE;
		else if (op_table[opcode].mode == mode_absolute)
			*mode_code = CPU6502_ADDR_MODE_ABSOLUTE_CODE;
		else if (op_table[opcode].mode == mode_absolute_x)
			*mode_code = CPU6502_ADDR_MODE_ABSOLUTE_X_CODE;
		else if (op_table[opcode].mode == mode_absolute_y)
			*mode_code = CPU6502_ADDR_MODE_ABSOLUTE_Y_CODE;
		else if (op_table[opcode].mode == mode_indirect)
			*mode_code = CPU6502_ADDR_MODE_INDIRECT_CODE;
		else if (op_table[opcode].mode == mode_indirect_x)
			*mode_code = CPU6502_ADDR_MODE_INDIRECT_X_CODE;
		else if (op_table[opcode].mode == mode_indirect_y)
			*mode_code = CPU6502_ADDR_MODE_INDIRECT_Y_CODE;
		else if (op_table[opcode].mode == mode_implied)
			*mode_code = CPU6502_ADDR_MODE_IMPLIED_CODE;
		else if (op_table[opcode].mode == mode_relative)
			*mode_code = CPU6502_ADDR_MODE_RELATIVE_CODE;
		else
			*mode_code = CPU6502_ADDR_MODE_ERROR_CODE;
	} else {
		strcpy(m_name, "UNKOWN_OPCODE");
		strcpy(m_mode, "UNKOWN_ADDR_MODE");
		*mode_code = CPU6502_ADDR_MODE_ERROR_CODE;
		return_status = false;
	}

	*name = m_name;
	*mode = m_mode;
	return return_status;
}

u8	cpu6502_rega(struct cpu6502 *cpu)
{
	return cpu->reg_a;
}

u8	cpu6502_regx(struct cpu6502 *cpu)
{
	return cpu->reg_x;
}

u8	cpu6502_regy(struct cpu6502 *cpu)
{
	return cpu->reg_y;
}

u8	cpu6502_regsp(struct cpu6502 *cpu)
{
	return cpu->reg_sp;
}

u8	cpu6502_regp(struct cpu6502 *cpu)
{
	return cpu->reg_p;
}

u16	cpu6502_regpc(struct cpu6502 *cpu)
{
	return cpu->reg_pc;
}
