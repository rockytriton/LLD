#include "gpio.h"
#include "peripherals/i2c.h"
#include "i2c.h"
#include "printf.h"

#define I2C_SPEED 100000

void i2c_init() {
    gpio_pin_set_func(2, GFAlt0);
    gpio_pin_set_func(3, GFAlt0);
    gpio_pin_enable(2);
    gpio_pin_enable(3);

    REGS_I2C->div = CORE_CLOCK_SPEED / I2C_SPEED;
}

i2c_status i2c_recv(u8 address, u8 *buffer, u32 size) {
    int count = 0;

    REGS_I2C->slave_address = address;
    REGS_I2C->control = C_CLEAR;
    REGS_I2C->status = S_CLKT | S_ERR | S_DONE;
    REGS_I2C->data_length = size;
    REGS_I2C->control = C_I2CEN | C_ST | C_READ;

    while(!(REGS_I2C->status & S_DONE)) {
        while(REGS_I2C->status & S_RXD) {
            *buffer++ = REGS_I2C->fifo & 0xFF;
            count++;
        }
    }

    while(count < size && REGS_I2C->status & S_RXD) {
        *buffer++ = REGS_I2C->fifo & 0xFF;
        count++;
    }

    reg32 status = REGS_I2C->status;

    REGS_I2C->status = S_DONE;

    if (status & S_ERR) {
        return I2CS_ACK_ERROR;
    } else if (status & S_CLKT) {
        return I2CS_CLOCK_TIMEOUT;
    } else if (count < size) {
        return I2CS_DATA_LOSS;
    }

    return I2CS_SUCCESS;
}

i2c_status i2c_send(u8 address, u8 *buffer, u32 size) {
    int count = 0;

    REGS_I2C->slave_address = address;
    REGS_I2C->control = C_CLEAR;
    REGS_I2C->status = S_CLKT | S_ERR | S_DONE;
    REGS_I2C->data_length = size;
    REGS_I2C->control = C_I2CEN | C_ST;

    while(!(REGS_I2C->status & S_DONE)) {
        while(count < size && REGS_I2C->status & S_TXD) {
            REGS_I2C->fifo = *buffer++;
            count++;
        }
    }

    reg32 status = REGS_I2C->status;

    REGS_I2C->status = S_DONE;

    if (status & S_ERR) {
        return I2CS_ACK_ERROR;
    } else if (status & S_CLKT) {
        return I2CS_CLOCK_TIMEOUT;
    } else if (count < size) {
        return I2CS_DATA_LOSS;
    }

    return I2CS_SUCCESS;
}
