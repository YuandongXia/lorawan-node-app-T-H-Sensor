/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __ADC_BOARD_H
#define __ADC_BOARD_H

#include <stdint.h>
#include "gpio.h"

#define ADC_FULL_SCALE              (4095)

/*!
 * I2C object type definition
 */
typedef struct
{
    PinNames Pin;
}adc_t;

void hal_adc_init(adc_t *obj, PinNames Pin);
void hal_adc_deinit( void );
uint16_t hal_adc_read(adc_t *obj);
uint16_t hal_adc_get_vdd( void );
uint16_t hal_adc_get_vol(adc_t *obj);

#endif
