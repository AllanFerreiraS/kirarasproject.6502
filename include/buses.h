#ifndef _BUSES_H_
#define _BUSES_H_

#include "./utils.h"
#include <stdio.h>

struct buses {
	u8	(*read_bus)	(void *emulator, u16 address);
	void	(*write_bus)	(void *emulator, u16 address, u8 data);
	void	*emulator;
	u8	previous_data;
};

u16  buses_mirror(u16 address, u16 address_init, u16 size);
u8 buses_addr_in_range(u16 addr, u16 min_addr, u16 max_addr);
u8   buses_openbus(struct buses *bus);
void buses_update_previous_data(struct buses *bus, u8 last_data);

#endif
