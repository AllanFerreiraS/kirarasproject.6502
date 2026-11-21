#include "./../include/buses.h"

u16 buses_mirror(u16 address, u16 address_init, u16 size)
{
	return (address % (size)) + address_init;
}

u8 buses_addr_in_range(u16 addr, u16 min_addr, u16 max_addr)
{
	return ((addr >= min_addr) && (addr <= max_addr));
}

u8 buses_openbus(struct buses *bus)
{
	printf("previous data: %02x\n", bus->previous_data);
	return bus->previous_data;
}

void buses_update_previous_data(struct buses *bus, u8 last_data)
{
	bus->previous_data = last_data;
}
