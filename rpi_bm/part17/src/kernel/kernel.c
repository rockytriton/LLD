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
#include <io.h>
#include "peripherals/emmc.h"

void putc(void *p, char c) {
    if (c == '\n') {
        uart_send('\r');
    }

    uart_send(c);
}

u32 get_el();


#define BOOT_SIGNATURE 0xAA55

typedef struct PACKED {
    u8 head;
    u8 sector : 6;
    u8 cylinder_hi : 2;
    u8 cylinder_lo;
} chs_address ;

typedef struct PACKED {
    u8 status;
    chs_address first_sector;
    u8 type;
    chs_address last_sector;
    u32 first_lba_sector;
    u32 num_sectors;
} partition_entry ;

typedef struct PACKED {
    u8 bootCode[0x1BE];
    partition_entry partitions[4];
    u16 bootSignature;
} master_boot_record;


void kernel_main() {
    uart_init();
    init_printf(0, io_device_find("muart"));
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

#if INIT_MMU == 1
    printf("Initialized MMU\n");
#endif

    if (!emmc_init()) {
        printf("FAILED TO INIT EMMC\n");
        return;
    }

    printf("EMMC Disk initialized\n");

    master_boot_record mbr;

    io_device *disk = io_device_find("disk");
    
    int r = disk->read(disk, &mbr, sizeof(mbr));

    printf("Read disk returned: %d\n", r);

    if (mbr.bootSignature != BOOT_SIGNATURE) {
        printf("BAD BOOT SIGNATURE: %X\n", mbr.bootSignature);
    }

    for (int i=0; i<4; i++) {
        if (mbr.partitions[i].type == 0) {
            break;
        }

        printf("Partition %d:\n", i);
        printf("\t Type: %d\n", mbr.partitions[i].type);
        printf("\t NumSecs: %d\n", mbr.partitions[i].num_sectors);
        printf("\t Status: %d\n", mbr.partitions[i].status);
        printf("\t Start: %X\n", mbr.partitions[i].first_lba_sector);
    }

    timer_sleep(15000);

    void *p1 = get_free_pages(10);
    void *p2 = get_free_pages(4);
    void *p3 = allocate_memory(20 * 4096 + 1);

    free_memory(p1);
    free_memory(p2);
    free_memory(p3);

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
