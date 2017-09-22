/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "adxl345.h"
#include "board.h"

int adxl345_write(uint8_t addr, uint8_t val)
{
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = val;
    if( hal_i2c_write(&i2c1, ADXL345_I2C_ADDR, (char const *)buf, 2, 1) < 0 ){
        return -1;
    }

    return 0;
}

int16_t adxl345_read(uint8_t addr)
{
    uint8_t reg, ret;

    reg = addr;
    if(hal_i2c_write(&i2c1, ADXL345_I2C_ADDR, (char const *)&reg, 1, 0) < 0){
        return -1;
    }

    if(hal_i2c_read(&i2c1, ADXL345_I2C_ADDR, (char *)&ret, 1, 1) < 0){
        return -1;
    }

    return ret;
}

int adxl345_read_buf(uint8_t addr, uint8_t *buf, int len)
{
    uint8_t reg;

    reg = addr;
    if( hal_i2c_write(&i2c1, ADXL345_I2C_ADDR, (char const *)&reg, 1, 0) < 0 ){
        return -1;
    }

    if( hal_i2c_read(&i2c1, ADXL345_I2C_ADDR, (char *)buf, len, 1) < 0 ){
        return -1;
    }

    return len;
}

int adxl345_set_range(uint8_t gRange, uint8_t fullRes)
{
    int16_t oldDataFormat = 0;
    unsigned char newDataFormat = 0;

    oldDataFormat = adxl345_read(ADXL345_DATA_FORMAT);
    if(oldDataFormat < 0){
        return -1;
    }

    newDataFormat = oldDataFormat & ~(ADXL345_RANGE(0x3) | ADXL345_FULL_RES);
    newDataFormat =  newDataFormat | ADXL345_RANGE(gRange) | fullRes;
    if(adxl345_write(ADXL345_DATA_FORMAT, newDataFormat)<0){
        return -1;
    }

    return 0;
}

/* 1: measure, 0: standby */
int adxl345_set_mode(uint8_t mode)
{
    uint8_t reg;

    reg = adxl345_read(ADXL345_POWER_CTL);
//    if( (mode && (reg&ADXL345_PCTL_MEASURE)) || (!mode && !(reg&ADXL345_PCTL_MEASURE)) ){
//        // already in meaure mode
//        return 0;
//    }

    if(mode){
        reg |= ADXL345_PCTL_MEASURE;
    }else{
        reg &= ~ADXL345_PCTL_MEASURE;
    }

    if( adxl345_write(ADXL345_POWER_CTL, reg)<0 ){
        return -1;
    }

    return 0;
}

uint8_t adxl345_get_id(void)
{
    return adxl345_read(ADXL345_DEVID);
}

/* buf size must larger than 6
    buf[0] --> X-MSBs
    buf[1] --> X-LSBs
    buf[2] --> Y-MSBs
    buf[3] --> Y-LSBs
    buf[4] --> Z-MSBs
    buf[5] --> Z-LSBs
    */
int adxl345_read_xyz(uint8_t *buf)
{
    uint8_t tmp;
    if(adxl345_read_buf(ADXL345_DATAX0, buf, 6)<0){
        return -1;
    }

    // swap MSBs and LSBs
    tmp = buf[0];
    buf[0] = buf[1];
    buf[1] = tmp;

    tmp = buf[2];
    buf[2] = buf[3];
    buf[3] = tmp;

    tmp = buf[4];
    buf[4] = buf[5];
    buf[5] = tmp;
    return 0;
}

