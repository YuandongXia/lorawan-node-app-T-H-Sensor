/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "loudness.h"
#include "board.h"
#include "adc-board.h"

adc_t ln_in;

void ln_init(void)
{
    hal_adc_init(&ln_in, ADC_IN);
    hal_adc_read(&ln_in);
}

uint16_t ln_read()
{
    uint16_t ret;

    ret = hal_adc_get_vdd()*hal_adc_read(&ln_in)/ADC_FULL_SCALE;

    return ret;
}
