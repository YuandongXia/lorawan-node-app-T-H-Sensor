/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __OLED_H
#define __OLED_H

#include "board.h"

void oled_putchar(uint8_t x, uint8_t y, char c);
void oled_putstr(uint8_t x, uint8_t y, char *str);

void oled_putchar_big(uint8_t x, uint8_t y, char c);
void oled_putstr_big(uint8_t x, uint8_t y, char *str);

void oled_putchar_huge(uint8_t x, uint8_t y, char c);
void oled_putstr_huge(uint8_t x, uint8_t y, char *str);

void oled_draw_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

#endif
