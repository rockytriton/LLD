#pragma once

#include "common.h"

void delay(u64 ticks);
void put32(u64 address, u32 value);
u32 get32(u64 address);
