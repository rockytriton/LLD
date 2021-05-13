#include "common.h"
#include "mini_uart.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "i2c.h"
#include "spi.h"
#include "led_display.h"

void putc(void *p, char c) {
    if (c == '\n') {
        uart_send('\r');
    }

    uart_send(c);
}

u32 get_el();


void kernel_main() {
    uart_init();
    init_printf(0, putc);
    printf("\nRasperry PI Bare Metal OS Initializing...\n");

    irq_init_vectors();
    enable_interrupt_controller();
    irq_enable();
    timer_init();

#if RPI_VERSION == 3
    printf("\tBoard: Raspberry PI 3\n");
#endif

#if RPI_VERSION == 4
    printf("\tBoard: Raspberry PI 4\n");
#endif

    printf("\nException Level: %d\n", get_el());

    printf("Sleeping 200 ms...\n");
    timer_sleep(200);

    printf("Initializing I2C...\n");
    i2c_init();

    for (u8 i=0x20; i<0x30; i++) {
        if (i2c_send(i, &i, 1) == I2CS_SUCCESS) {
            //we know there is an i2c device here now.
            printf("Found device at address 0x%X\n", i);
        }
    }

    printf("Initializing SPI...\n");
    spi_init();

    printf("Initializing Display...\n");
    led_display_init();
    timer_sleep(2000);

    led_display_clear();

    printf("Cleared\n");
    
    for (int i=0; i<=9; i++) {
        for (int d=0; d<8; d++) {
            led_display_set_digit(d, i, false);
            timer_sleep(200);
        }
    }

    printf("Intensifying...\n");

    for (int i=0; i<16; i++) {
        printf("Intensity: %d\n", i);
        led_display_intensity(i);
        timer_sleep(200);
    }

    led_display_clear();
    timer_sleep(2000);

    //HELLO
    led_display_send_command(LD_DIGIT4, 0b00110111);
    led_display_send_command(LD_DIGIT3, 0b01001111);
    led_display_send_command(LD_DIGIT2, 0b00001110);
    led_display_send_command(LD_DIGIT1, 0b00001110);
    led_display_send_command(LD_DIGIT0, 0b01111110);


    printf("Shutting down...\n");
    timer_sleep(2000);
    led_display_send_command(LD_SHUTDOWN, 0);

    printf("DONE!\n");

    while(1) {
        //uart_send(uart_recv());
    }
}
