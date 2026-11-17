#ifndef _BUSES_H_
#define _BUSES_H_

#include "./utils.h"
#include <stdint.h>

typedef struct {
	u8	(*read_bus)	(void *emulator, u16 address);
	void	(*write_bus)	(void *emulator, u16 address, u8 data);
	void	*emulator;
} buses;

#endif
