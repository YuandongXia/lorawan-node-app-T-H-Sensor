/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __LED_H
#define __LED_H

#include <stdint.h>
#include "board.h"

typedef enum{
	LED0 = 0,
#if (LED_MAX_NUMS > 1)
	LED1,
#endif
#if (LED_MAX_NUMS > 2)
	LED2,
#endif
#if (LED_MAX_NUMS > 3)
	LED3,
#endif
#if (LED_MAX_NUMS > 4)
	LED4,
#endif
#if (LED_MAX_NUMS > 5)
	LED5,
#endif
}led_index_t;

void led_init(void);
void led_on(led_index_t led_index);
void led_off(led_index_t led_index);
void led_toggle(led_index_t led_index);
void led_blink(led_index_t led_index, uint32_t light_on_time);

#endif
