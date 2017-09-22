/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __HAL_I2C_H
#define __HAL_I2C_H

typedef struct
{
    void *I2c;
    Gpio_t Scl;
    Gpio_t Sda;
}i2c_t;

typedef I2C_TypeDef hal_i2c_t;

extern i2c_t i2c1;

int hal_i2c_init(i2c_t *obj, PinNames scl, PinNames sda);
void hal_i2c_deinit(i2c_t *obj, PinNames scl, PinNames sda);
int hal_i2c_reset(i2c_t *obj);
void hal_i2c_frequency(i2c_t *obj, int hz);
int hal_i2c_read(i2c_t *obj, int address, char *data, int length, int stop);
int hal_i2c_write(i2c_t *obj, int address, const char *data, int length, int stop);
int hal_i2c_byte_write(i2c_t *obj, int data);
int hal_i2c_byte_read(i2c_t *obj, int last);

#endif // __I2C_MCU_H__
