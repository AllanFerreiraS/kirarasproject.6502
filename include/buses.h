#ifndef _BUSES_H_
#define _BUSES_H_

#include <stdint.h>

typedef struct {
	uint8_t	(*read_fn)	(void *infra, uint16_t address);
	void	(*write_fn)	(void *infra, uint16_t address, uint8_t data);
	void	*infra;
} buses;

#endif
