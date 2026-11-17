#ifndef _CPU6502_H_
#define _CPU6502_H_

#include "./../buses.h"
#include "./../utils.h"
#include <stdint.h>

// Stack Pointer Start Index
#define CPU6502_STACK_LOWER_ADDR		0x100

// Interrupts Low Addresses
#define CPU6502_INTERRUPT_IRQ_LW_ADDR		0xFFFE
#define CPU6502_INTERRUPT_NMI_LW_ADDR		0xFFFA
#define CPU6502_INTERRUPT_RESET_LW_ADDR		0xFFFC

// Status Pointer Flags
#define CPU6502_STATUS_FLAG_NEG			0x80	// Negative
#define CPU6502_STATUS_FLAG_OVF			0x40	// Overflow
#define CPU6502_STATUS_FLAG_NOEFFECT		0x20	// Literally no effect on the processor
#define CPU6502_STATUS_FLAG_BRK			0x10	// Break
#define CPU6502_STATUS_FLAG_DEC			0x08	// Decimal
#define CPU6502_STATUS_FLAG_INT			0x04	// Interrupt
#define CPU6502_STATUS_FLAG_ZER			0x02	// Zero
#define CPU6502_STATUS_FLAG_CAR			0x01	// Carry
#define CPU6502_STATUS_FLAG_INITIAL_STATE	0x24	// NOEFFECT and INT FLAGS are NOT set

#define CPU6502_FLAG_READ			0x00
#define CPU6502_FLAG_WRITE			0x01

// Interrupt Pins
#define CPU6502_RESET				0x00
#define CPU6502_POWER_UP			0x01
#define CPU6502_IRQ_LOW				0x00
#define CPU6502_IRQ_HIGH			0x01
#define CPU6502_NMI_LOW				0x00
#define CPU6502_NMI_HIGH			0x01

// Addresses Mode Arbitrary Codes
#define CPU6502_ADDR_MODE_IMMEDIATE_CODE	0x00
#define CPU6502_ADDR_MODE_ZERO_PAGE_CODE	0x01
#define CPU6502_ADDR_MODE_ZERO_PAGE_X_CODE	0x02
#define CPU6502_ADDR_MODE_ZERO_PAGE_Y_CODE	0x03
#define CPU6502_ADDR_MODE_ABSOLUTE_CODE		0x04
#define CPU6502_ADDR_MODE_ABSOLUTE_X_CODE	0x05
#define	CPU6502_ADDR_MODE_ABSOLUTE_Y_CODE	0x06
#define CPU6502_ADDR_MODE_INDIRECT_CODE		0x07
#define CPU6502_ADDR_MODE_INDIRECT_X_CODE	0x08
#define CPU6502_ADDR_MODE_INDIRECT_Y_CODE	0x09
#define CPU6502_ADDR_MODE_IMPLIED_CODE		0x10
#define CPU6502_ADDR_MODE_RELATIVE_CODE		0x11
#define CPU6502_ADDR_MODE_ERROR_CODE		0x12

// Others
#define CPU6502_OPCODES_AMOUNT			0x100

struct cpu6502 {
	u8	reg_a;		// A
	u8 	reg_x;		// X
	u8 	reg_y;		// Y
	u8	reg_sp;		//Stack Pointer
	u8	reg_p;		//Status Register. Bits: 7:Negative | 6:Overflow | 5:Always 1 | 4:Break Flag | 3:Decimal | 2:Interrupt Disable | 1:Zero | 0:Carry
	u8	read_write;
	u16	reg_pc;		//Program Counter

	u8 	irq_pin;	// Low or High Values
	u8	nmi_pin;	// Low or High Values
	u8	nmi_previous;	// Low or High Values
	u8	nmi_request;	// TRUE or FALSE
	u64	cycles;
	buses	bus;
};

void	cpu6502_init(struct cpu6502 *processor, u8 power_up);
void 	cpu6502_init_manual(struct cpu6502 *processor, u16 reg_pc);
void 	cpu6502_opcode_execute(struct cpu6502 *processor);
void	cpu6502_interrupt_handle_request_if_any(struct cpu6502 *processor);
u8	cpu6502_opcode_get_infos(u8 opcode, char **name, char **mode, u8 *mode_code);

#endif
