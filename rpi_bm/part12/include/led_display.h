#pragma once

#include "common.h"

void led_display_init();

void led_display_send_command(u8 opcode, u8 data);

void led_display_clear();

void led_display_intensity(u8 value);

void led_display_set_digit(u8 digit, u8 value, bool dot);

void led_display_set_value(int value);

typedef enum {
    LD_NOOP = 0,
    LD_DIGIT0,
    LD_DIGIT1,
    LD_DIGIT2,
    LD_DIGIT3,
    LD_DIGIT4,
    LD_DIGIT5,
    LD_DIGIT6,
    LD_DIGIT7,
    LD_DECODE_MODE,
    LD_INTENSITY,
    LD_SCANLIMIT,
    LD_SHUTDOWN,
    LD_DISPLAYTEST = 15
} led_display_opcodes;

