#pragma once

#include "common.h"

typedef enum _i2c_status {
    I2CS_SUCCESS,
    I2CS_ACK_ERROR,
    I2CS_DATA_LOSS,
    I2CS_CLOCK_TIMEOUT
} i2c_status;

void i2c_init();

i2c_status i2c_recv(u8 address, u8 *buffer, u32 size);

i2c_status i2c_send(u8 address, u8 *buffer, u32 size);



