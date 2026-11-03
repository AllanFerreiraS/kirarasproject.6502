#ifndef _RAW_H_
#define _RAW_H_

#include "./../../include/infrastructure.h"
#include "./../../include/buses.h"
#include "./../../include/debugger.h"

uint8_t bus_raw_read(void *infra, uint16_t address);
void 	bus_raw_write(void *infra, uint16_t address, uint8_t data);
void 	raw_run(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t pc_start, uint16_t memory_start);
void 	raw_run_with_monitor(infra_raw * e, uint8_t step_by_step, const char * bin, uint16_t pc_start, uint16_t memory_start);
#endif
