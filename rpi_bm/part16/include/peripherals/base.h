#pragma once

#include "mm.h"


#if RPI_VERSION == 3
#define DEVICE_BASE 		0x3F000000	

#elif RPI_VERSION == 4
#define DEVICE_BASE 		0xFE000000	
//#define DEVICE_BASE 		0xFE000000	

#else
#define PBASE 0
#error RPI_VERSION NOT DEFINED

#endif

#define CORE_CLOCK_SPEED 1500000000

#define PBASE 			(VA_START + DEVICE_BASE)
