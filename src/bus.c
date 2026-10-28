#include "./../include/bus.h"
#include "./../include/processor.h"
#include "./../include/memory_16.h"
#include <stdint.h>

typedef struct {
	uint8_t 	hardware_id;
	const char	*hardware_name;
	uint16_t	hardware_addr_index;
	uint16_t	hardware_addr_len;
} address_table;

uint8_t bus_read(processor_6502 * processor, uint16_t address)
{
	memory_read();
}
