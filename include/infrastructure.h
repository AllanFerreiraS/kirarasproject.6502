#ifndef _INFRASTRUCTURE_H_
#define _INFRASTRUCTURE_H_

#include "./hardwares/processor_6502.h"
#include "./hardwares/memory_16.h"

typedef struct {
	processor_6502 	cpu;
	memory_16	memory;
} infra_raw;

#endif
