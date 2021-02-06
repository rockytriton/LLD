#pragma once

#include "common.h"

//HD44780 Datasheet
//https://image.dfrobot.com/image/data/DFR0154/LCD2004%20hd44780%20Datasheet.pdf

void lcd_init(u8 address);

void lcd_backlight(bool on);

void lcd_print(char *s);

void lcd_send(u8 data, u8 mode);

void lcd_command(u8 command);
