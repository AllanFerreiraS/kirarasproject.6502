#ifndef _RAW_H_
#define _RAW_H_

#include "./../../include/infrastructure.h"

uint8_t bus_raw_read(void *infra, uint16_t address);
void 	bus_raw_write(void *infra, uint16_t address, uint8_t data);
void 	raw_run(infra_raw * e);

#endif
