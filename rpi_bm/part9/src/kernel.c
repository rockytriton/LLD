#include "common.h"
#include "mini_uart.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"

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

    printf("Sleeping 200 ms...\n");
    timer_sleep(200);

    printf("Sleeping 200 ms...\n");
    timer_sleep(200);

    printf("Sleeping 2 seconds...\n");
    timer_sleep(2000);

    printf("Sleeping 2 seconds...\n");
    timer_sleep(2000);

    printf("Sleeping 5 seconds...\n");
    timer_sleep(5000);

    printf("DONE!\n");

    while(1) {
        //uart_send(uart_recv());
    }
}
