#include "common.h"
#include "mini_uart.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "i2c.h"

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

    printf("Initializing LCD...\n");
    lcd_init(0x27);

    for (int i=0; i<5; i++) {
        lcd_backlight(true);
        timer_sleep(250);
        lcd_backlight(false);
        timer_sleep(250);
    }

    lcd_backlight(true);
    lcd_print("Hello LCD!");


    printf("DONE!\n");

    while(1) {
        //uart_send(uart_recv());
    }
}
