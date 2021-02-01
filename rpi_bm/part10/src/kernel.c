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

    for (int i=0; i<10; i++) {
        char buffer[10];
        i2c_recv(21, buffer, 9);
        buffer[9] = 0;

        printf("Received: %s\n", buffer);

        timer_sleep(250);
    }

    for (u8 d=0; d<20; d++) {
        i2c_send(21, &d, 1);

        timer_sleep(250);

        printf("Sent: %d\n", d);
    }

    char *msg = "Hello Slave Device";
    i2c_send(21, msg, 18);


    printf("DONE!\n");

    while(1) {
        //uart_send(uart_recv());
    }
}
