/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "th02.h"
#include "board.h"

int th02_write(uint8_t addr, uint8_t val)
{
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = val;
    if( hal_i2c_write(&i2c1, TH02_I2C_ADDR, (char const *)buf, 2, 1) < 0 ){
        return -1;
    }

    return 0;
}

int16_t th02_read(uint8_t addr)
{
    uint8_t reg, ret;

    reg = addr;
    if(hal_i2c_write(&i2c1, TH02_I2C_ADDR, (char const *)&reg, 1, 0) < 0){
        return -1;
    }

    if(hal_i2c_read(&i2c1, TH02_I2C_ADDR, (char *)&ret, 1, 1) < 0){
        return -1;
    }

    return ret;
}

int32_t th02_read_word(uint8_t addr)
{
    uint8_t reg;
    uint8_t buf[2];
    uint16_t ret;

    reg = addr;
    if( hal_i2c_write(&i2c1, TH02_I2C_ADDR, (char const *)&reg, 1, 0) < 0 ){
        return -1;
    }

    if( hal_i2c_read(&i2c1, TH02_I2C_ADDR, (char *)buf, 2, 1) < 0 ){
        return -1;
    }
    ret = ((uint16_t)buf[0]<<8) + buf[1];
    return ret;
}

uint8_t th02_get_id(void)
{
    return th02_read(TH02_ID);
}

int th02_wait2rdy(void)
{
    int16_t sta;
    do{
        sta = th02_read(TH02_STATUS);
    }while( (sta>=0) && (sta&0x01) != 0 );

    if(sta < 0){
        return -1;
    }
    return 0;
}

/** temp = return_value/32-50 */
uint32_t th02_get_temp(void)
{
    int32_t ret;

    if( th02_write(TH02_CONFIG, TH02_CONFIG_START|TH02_CONFIG_TEMP) < 0 ){
        return 0;
    }

    if( th02_wait2rdy() < 0 ){
        return 0;
    }
    ret = th02_read_word(TH02_DATAH);
    if(ret<0){
        return 0;
    }

    return (ret>>2);
}

/** %RH = return_value/16-24 */
uint32_t th02_get_rh(void)
{
    int32_t ret;

    if( th02_write(TH02_CONFIG, TH02_CONFIG_START|TH02_CONFIG_HUMI) < 0 ){
        return 0;
    }

    if( th02_wait2rdy() < 0 ){
        return 0;
    }
    ret = th02_read_word(TH02_DATAH);
    if(ret<0){
        return 0;
    }

    return (ret>>4);
}

