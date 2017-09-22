/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

*/
#ifndef __SHT21_H
#define __SHT21_H

#include <stdint.h>

// SHT21 I2C Device address
#define SHT21_I2C_ADDR              (0x40<<1)

#define SHT3X_I2C_VSS_ADDR          (0x44<<1)
#define SHT3X_I2C_VCC_ADDR          (0x45<<1)
#define SHT3X_I2C_ADDR              (SHT3X_I2C_VSS_ADDR)

// SHT3X work in Clock stretching disabled, low Repeatability
#define SHT3X_NOCLKSTR_LOWREP_MSB   (0x24)
#define SHT3X_NOCLKSTR_LOWREP_LSB   (0x00)

#define SHT3X_PERIODIC_MSB          (0x20)
#define SHT3X_PERIODIC_LSB          (0x32)

#define SHT3X_PERIODIC_READ_MSB     (0xE0)
#define SHT3X_PERIODIC_READ_LSB     (0x00)

#define SHT21_REG_THOLD             (0xE3)
#define SHT21_REG_HHOLD             (0xE5)
#define SHT21_REG_TNHOLD            (0xF3)
#define SHT21_REG_HNHOLD            (0xF5)
#define SHT21_REG_WRITE             (0xE6)
#define SHT21_REG_READ              (0xE7)
#define SHT21_REG_RST               (0xEF)

typedef enum{
    SHTNONE = 0,
    SHT2X = 1,
    SHT3X = 2,
}sht_t;

typedef enum{
    SHT_FMT_SHT21,      // SHT21: T=175.72*temp/2^16-46.85, RH=125*rh/2^16-6
    SHT_FMT_STD,        // Standard: T=temp/400-50, RH=rh/2
    SHT_FMT_STEE,
    SHT_FMT_DIR,         //Direct reading: T = temp/100, RH=rh/100
}sht_fmt_t;

sht_t shtxx_read_th(sht_fmt_t fmt, uint16_t *temp,uint16_t * rh);
sht_t shtxx_read_th_float(float *temp, float *rh);
uint16_t shtxx_error(void);

#endif
