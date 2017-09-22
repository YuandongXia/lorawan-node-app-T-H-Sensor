/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __TH02_H
#define __TH02_H

#include <stdint.h>

// TH02 I2C Device address
#define TH02_I2C_ADDR           (0x40<<1)

// TH02 Registers addresses
#define TH02_STATUS             (0)
#define TH02_DATAH              (1)
#define TH02_DATAL              (2)
#define TH02_CONFIG             (3)
#define TH02_ID                 (17)

// Unititialized values (arbitrary)
#define TH02_UNINITIALIZED_TEMP 55555 // int32_t internal value
#define TH02_UNINITIALIZED_RH   1111  // int32_t internal value
#define TH02_UNDEFINED_VALUE    12345 // int16_t returned value

// we decide error if conversion is >= 50ms
#define TH02_CONVERSION_TIME_OUT  50

// Bit definition of TH02 registers values
#define TH02_STATUS_RDY    0x01

#define TH02_CONFIG_START  0x01
#define TH02_CONFIG_HEAT   0x02
#define TH02_CONFIG_TEMP   0x10
#define TH02_CONFIG_HUMI   0x00
#define TH02_CONFIG_FAST   0x20

// THO2 Linearization Coefficients
#define TH02_A0   -4.7844
#define TH02_A1    0.4008
#define TH02_A2   -0.00393

// TH02 Temperature compensation Linearization Coefficients
#define TH02_Q0   0.1973
#define TH02_Q1   0.00237

uint8_t th02_get_id(void);
uint32_t th02_get_temp(void);
uint32_t th02_get_rh(void);

#endif
