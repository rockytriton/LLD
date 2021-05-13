#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h"

const uint BASE_BUS_ADDRESS = 0x8000;
const uint BASE_MEM_ADDRESS = 0x1000;

#define I2C_PORT i2c0
#define I2C_ADDR 0x20

//8 bytes of RAM
static unsigned char MEM[] = {0, 0, 0, 0, 0, 0, 0, 0};

#define ADDR_TO_2BYTES(a) (a & 0xFF), ((a >> 8) & 0xFF)

//CODE to run on the 6502 processor.
static unsigned char CODE[] = {
    0xA0, 0x00, //LDY $00
    0x18, //CLC
    0xB8, //CLV
    0xA9, 0xF0, //LDA $F0
    0x8D, ADDR_TO_2BYTES(BASE_MEM_ADDRESS), //STA 0x1000
    0x6A, //ROR
    0x8D, ADDR_TO_2BYTES(BASE_MEM_ADDRESS), //STA 0x1000
    0x6A, //ROR
    0x8D, ADDR_TO_2BYTES(BASE_MEM_ADDRESS), //STA 0x1000
    0x6A, //ROR
    0x8D, ADDR_TO_2BYTES(BASE_MEM_ADDRESS), //STA 0x1000
    0x6A, //ROR
    0x8D, ADDR_TO_2BYTES(BASE_MEM_ADDRESS), //STA 0x1000
    0xAA, //TAX //transfer A to X
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0x8E, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 1), //STX 0x1001
    0xE8, //INX  (x++)
    0xC8, //INY  (y++)
    0x8C, ADDR_TO_2BYTES(BASE_MEM_ADDRESS + 2), //STY 0x1002
    0x4C, ADDR_TO_2BYTES(BASE_BUS_ADDRESS + 4)
};

const uint CLOCK = 9;
const uint RW = 8;

int addr_pins[] = {
    16, 17, 18, 19,
    20, 21, 22, 26,
    27, 28, 15, 14,
    13, 12, 11, 10
};

void write_data(uint8_t val) {
    if (i2c_write_blocking(I2C_PORT, I2C_ADDR, &val, 1, false) <= 0) {
        printf("FAILED TO WRITE TO I2C\n");
    }
}

bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void scan_bus() {
    sleep_ms(5000);
    printf("I2C Bus Scan...\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    
    for (int addr = 0; addr<(1 << 7); addr++) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr)) {
            ret = PICO_ERROR_GENERIC;
        } else {
            ret = i2c_read_blocking(i2c0, addr, &rxdata, 1, false);
        }

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }

    printf("DONE\n");
    sleep_ms(2000);
}

void asm_test();
void asm_test2();

int nothing() {
    return 5;
}

int main() {
    int counter = 0;

    //asm_test();

    counter++;
    counter++;
    counter = nothing();
    counter++;

    //asm_test2();

    i2c_init(I2C_PORT, 100 * 1000);

    //page 26
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);

    bi_decl(bi_2pins_with_func(4, 5, GPIO_FUNC_I2C));

    gpio_init(CLOCK);
    gpio_set_dir(CLOCK, GPIO_OUT);

    gpio_init(RW);
    gpio_set_dir(RW, GPIO_IN);

    for (int i=0; i<16; i++) {
        gpio_init(addr_pins[i]);
        gpio_set_dir(addr_pins[i], GPIO_IN);
    }

    stdio_init_all();

    scan_bus();

    while(true) {
        //pulse clock
        uint8_t val = 0xEA; //NOOP by default..  page 22

        gpio_put(CLOCK, 0); //set clock LOW

        sleep_ms(10);

        bool isread = gpio_get(RW);

        unsigned int fullAddr = 0;
        for (int i=0; i<16; i++) {
            int bit = (gpio_get(addr_pins[i]) ? 1 : 0);
            fullAddr |= (bit << i);
        }

        bool is_pico_bus = fullAddr >= BASE_BUS_ADDRESS;

        if (!isread || !is_pico_bus) {
            write_data(0xFF); //set pins HIGH on the PCF8574 before reading values.
        }

        //6502 startup vector = 0xFFFC (2 bytes)
        if (fullAddr == 0xFFFC) {
            val = BASE_BUS_ADDRESS & 0xFF;
            write_data(val);
        } else if (fullAddr == 0xFFFD) {
            val = (BASE_BUS_ADDRESS >> 8) & 0xFF;
            write_data(val);
        }

        //check if the address is for code.
        if (fullAddr >= BASE_BUS_ADDRESS && fullAddr < BASE_BUS_ADDRESS + sizeof(CODE)) {
            val = CODE[fullAddr - BASE_BUS_ADDRESS];
            write_data(val);
        }

        //check if the address is for memory region.
        bool is_mem_addr = fullAddr >= BASE_MEM_ADDRESS && fullAddr < BASE_MEM_ADDRESS + sizeof(MEM);

        if (is_mem_addr && isread) {
            val = MEM[fullAddr - BASE_MEM_ADDRESS];
            write_data(val);
        }

        sleep_ms(5);

        gpio_put(CLOCK, 1); //Set clock HIGH

        sleep_ms(5);

        if (!isread || !is_pico_bus) {
            val = 0;
            int n = i2c_read_blocking(I2C_PORT, I2C_ADDR, &val, 1, false);

            if (n <= 0) {
                printf("FAILED READ: %2.2X\n", n);
            }
        }

        if (is_mem_addr && !isread) {
            MEM[fullAddr - BASE_MEM_ADDRESS] = val;
        }

        printf("ADDR: %4.4X - DATA: %2.2X - DIR: %c  ", 
                fullAddr, val, isread ? 'r' : 'W');

        if (!isread) {
            //print out the bits read that the processor wrote to the bus.
            for (int i=7; i >= 0; i--) {
                printf("%d", (val >> i) & 1);
            }
        } else {
            printf("        ");
        }

        printf("  MEM: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X \n", 
            MEM[0], MEM[1], MEM[2], MEM[3], MEM[4], MEM[5], MEM[6], MEM[7]);

        sleep_ms(250);
    }
}
