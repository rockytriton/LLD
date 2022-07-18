#include "led_display.h"
#include "spi.h"

const static u8 DIGIT_TABLE [] = {
    0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b01011111,0b01110000,
    0b01111111,0b01111011
};

void led_display_init() {
    led_display_send_command(LD_SCANLIMIT, 7);
    led_display_send_command(LD_DECODE_MODE, 0);
    led_display_send_command(LD_SHUTDOWN, 1);
    led_display_send_command(LD_INTENSITY, 0);
    led_display_send_command(LD_DISPLAYTEST, 0);
    led_display_intensity(5);
}

void led_display_send_command(u8 opcode, u8 data) {
    u8 cmd[2];
    cmd[0] = opcode;
    cmd[1] = data;

    spi_send(0, cmd, 2);
}

void led_display_clear() {
    for (int i=LD_DIGIT0; i<=LD_DIGIT7; i++) {
        led_display_send_command(i, 0);
    }
}

void led_display_intensity(u8 value) {
    led_display_send_command(LD_INTENSITY, value);
}

void led_display_set_digit(u8 digit, u8 value, bool dot) {
    u8 digit_value = DIGIT_TABLE[value];

    if (dot) {
        digit_value |= 0x80;
    }

    led_display_send_command(LD_DIGIT0 + digit, digit_value);
}

