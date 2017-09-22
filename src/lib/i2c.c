/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "i2c-board.h"

int hal_i2c_init(i2c_t *obj, PinNames scl, PinNames sda);
int hal_i2c_reset(i2c_t *obj);
void hal_i2c_frequency(i2c_t *obj, int hz);
int hal_i2c_read(i2c_t *obj, int address, char *data, int length, int stop);
int hal_i2c_write(i2c_t *obj, int address, const char *data, int length, int stop);
int hal_i2c_byte_write(i2c_t *obj, int data);
int hal_i2c_byte_read(i2c_t *obj, int last);


void i2c_init(i2c_t *obj, PinNames scl, PinNames sda)
{
    hal_i2c_init(obj, scl, sda);
}

void i2c_write()
{

}

void i2c_read()
{

}

