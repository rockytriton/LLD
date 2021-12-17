#include "common.h"
#include "mini_uart.h"
#include "printf.h"
#include "irq.h"
#include "timer.h"
#include "i2c.h"
#include "spi.h"
#include "led_display.h"
#include "mailbox.h"
#include "video.h"
#include "peripherals/base.h"

void putc(void *p, char c) {
    if (c == '\n') {
        uart_send('\r');
    }

    uart_send(c);
}

u32 get_el();

/*
#define PACKED __attribute((__packed__))
#define BOOT_SIGNATURE 0xAA55

volatile struct chs_address {
    u8 head;
    u8 sector : 6;
    u8 cylinder_hi : 2;
    u8 cylinder_lo;
} PACKED;

volatile struct partition_entry {
    u8 status;
    u8 first_head;
    u8 first_sector : 6;
    u8 first_cylinder_hi : 2;
    u8 first_cylinder_lo;

    u8 type;
    u8 last_head;
    u8 last_sector : 6;
    u8 last_cylinder_hi : 2;
    u8 last_cylinder_lo;

    u32 first_lba_sector;
    u32 num_sectors;

} PACKED;

volatile struct master_boot_record {
    u8 boot_code[0x1BE];
    struct partition_entry partitions[4];
    u16 boot_signature;
} PACKED;

volatile struct align_check {
    u8 a;
    u8 b;
    u8 c;
    u32 d;
} PACKED;
*/

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
/*
    printf("ALIGN CHECK\n");

    struct align_check ac;
    ac.d = 32;

    printf("ACD: %d\n", ac.d);

    printf("BOOT SIG CHECK\n");

    struct master_boot_record mbr;
    mbr.boot_signature = BOOT_SIGNATURE;
    mbr.partitions[0].first_lba_sector = 2112;

    printf("LBA: %d\n", mbr.partitions[0].first_lba_sector);
*/
    printf("\nException Level: %d\n", get_el());

    printf("Base Address: %lX\n", PBASE);

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

    printf("MAILBOX:\n");

    printf("CORE CLOCK: %d\n", mailbox_clock_rate(CT_CORE));
    printf("EMMC CLOCK: %d\n", mailbox_clock_rate(CT_EMMC));
    printf("UART CLOCK: %d\n", mailbox_clock_rate(CT_UART));
    printf("ARM  CLOCK: %d\n", mailbox_clock_rate(CT_ARM));

    printf("I2C POWER STATE:\n");

    for (int i=0; i<3; i++) {
        bool on = mailbox_power_check(i);

        printf("POWER DOMAIN STATUS FOR %d = %d\n", i, on);
    }

    //timer_sleep(2000);

    for (int i=0; i<3; i++) {
        u32 on = 1;
        mailbox_generic_command(RPI_FIRMWARE_SET_DOMAIN_STATE, i, &on);

        printf("SET POWER DOMAIN STATUS FOR %d = %d\n", i, on);
    }

    //timer_sleep(1000);

    for (int i=0; i<3; i++) {
        bool on = mailbox_power_check(i);

        printf("POWER DOMAIN STATUS FOR %d = %d\n", i, on);
    }

    u32 max_temp = 0;

    mailbox_generic_command(RPI_FIRMWARE_GET_MAX_TEMPERATURE, 0, &max_temp);

    printf("Got max temp: %d\n", max_temp);

    //Do video...
    video_init();

    printf("NO DMA...\n");
    video_set_dma(false);

    printf("Resolution 1900x1200\n");
    video_set_resolution(1900, 1200, 32);

    printf("Resolution 1024x768\n");
    video_set_resolution(1024, 768, 32);

    printf("Resolution 800x600\n");
    video_set_resolution(800, 600, 32);

    printf("Resolution 1900x1200\n");
    video_set_resolution(1900, 1200, 8);

    printf("Resolution 1024x768\n");
    video_set_resolution(1024, 768, 8);

    printf("Resolution 800x600\n");
    video_set_resolution(800, 600, 8);

    printf("YES DMA...\n");
    video_set_dma(true);

    printf("Resolution 1900x1200\n");
    video_set_resolution(1900, 1200, 32);

    printf("Resolution 1024x768\n");
    video_set_resolution(1024, 768, 32);

    printf("Resolution 800x600\n");
    video_set_resolution(800, 600, 32);

    printf("Resolution 1900x1200\n");
    video_set_resolution(1900, 1200, 8);

    printf("Resolution 1024x768\n");
    video_set_resolution(1024, 768, 8);

    printf("Resolution 800x600\n");
    video_set_resolution(800, 600, 8);

    while(1) {
        u32 cur_temp = 0;

        mailbox_generic_command(RPI_FIRMWARE_GET_TEMPERATURE, 0, &cur_temp);

        printf("Cur temp: %dC MAX: %dC\n", cur_temp / 1000, max_temp / 1000);

        timer_sleep(1000);
    }
}
