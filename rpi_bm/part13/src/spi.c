#include "spi.h"
#include "peripherals/spi.h"
#include "gpio.h"
#include "printf.h"

void spi_init() {
    gpio_pin_set_func(7, GFAlt0); //CS1
    gpio_pin_set_func(8, GFAlt0); //CS0  -> CS  (red)
    gpio_pin_set_func(9, GFAlt0); //MISO 
    gpio_pin_set_func(10, GFAlt0);//MOSI -> DIN (brown)
    gpio_pin_set_func(11, GFAlt0);//SCLK -> CLK (orange)
    gpio_pin_enable(7);
    gpio_pin_enable(8);
    gpio_pin_enable(9);
    gpio_pin_enable(10);
    gpio_pin_enable(11);
}

void spi_send_recv(u8 chip_select, u8 *sbuffer, u8 *rbuffer, u32 size) {
    REGS_SPI0->data_length = size;
    REGS_SPI0->cs = (REGS_SPI0->cs & ~CS_CS) | (chip_select << CS_CS__SHIFT) |
        CS_CLEAR_RX | CS_CLEAR_TX | CS_TA;
    
    u32 read_count = 0;
    u32 write_count = 0;

    while(read_count < size || write_count < size) {
        while(write_count < size && REGS_SPI0->cs & CS_TXD) {
            if (sbuffer) {
                REGS_SPI0->fifo = *sbuffer++;
            } else {
                REGS_SPI0->fifo = 0;
            }

            write_count++;
        }

        while(read_count < size && REGS_SPI0->cs & CS_RXD) {
            u32 data = REGS_SPI0->fifo;

            if (rbuffer) {
                *rbuffer++ = data;
            }

            read_count++;
        }
    }

    while(!(REGS_SPI0->cs & CS_DONE)) {
        while(REGS_SPI0->cs & CS_RXD) {
            u32 r = REGS_SPI0->fifo;
            printf("Left Over: %d\n", r);
        }
    }

    REGS_SPI0->cs = (REGS_SPI0->cs & ~CS_TA);
}

void spi_send(u8 chip_select, u8 *data, u32 size) {
    spi_send_recv(chip_select, data, 0, size);
}

void spi_recv(u8 chip_select, u8 *data, u32 size) {
    spi_send_recv(chip_select, 0, data, size);
}
