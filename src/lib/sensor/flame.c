/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "flame.h"
#include "board.h"
#include "adc-board.h"

Gpio_t flame;

void flame_init(void)
{
    GpioInit( &flame, ADC_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void flame_deinit(void)
{
    GpioInit( &flame, ADC_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

bool flame_detect()
{
    if(GpioRead(&flame) == 0){
        DelayMs(2);
        if(GpioRead(&flame) == 0){
            return true;
        }
    }
    return false;
}
