#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define UNUSED(x)	(void)(x)	// Unused variables

#define U8_SIZE		0x100
#define U16_SIZE	0x10000
#define U32_SIZE	0x100000000
#define U64_SIZE	0x10000000000000000

typedef uint8_t		u8;
typedef	uint16_t	u16;
typedef uint32_t	u32;
typedef	uint64_t	u64;
typedef int8_t		i8;
typedef	int16_t		i16;
typedef	int32_t		i32;
typedef	int64_t		i64;

#endif
