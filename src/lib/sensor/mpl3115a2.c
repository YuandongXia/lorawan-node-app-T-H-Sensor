/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "mpl3115a2.h"
#include "board.h"

#define MPL3115A2_OSR           (7<<3)

int16_t mpl3115a2_read(uint8_t addr)
{
    uint8_t reg, ret;

    reg = addr;
    if(hal_i2c_write(&i2c1, MPL3115A_I2C_ADDRESS, (char const *)&reg, 1, 0) < 0){
        return -1;
    }

    if(hal_i2c_read(&i2c1, MPL3115A_I2C_ADDRESS, (char *)&ret, 1, 1) < 0){
        return -1;
    }

    return ret;
}

int mpl3115a2_write(uint8_t addr, uint8_t val)
{
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = val;
    if( hal_i2c_write(&i2c1, MPL3115A_I2C_ADDRESS, (char const *)buf, 2, 1) < 0 ){
        return -1;
    }

    return 0;
}

int mpl3115a2_read_buf(uint8_t addr, uint8_t *buf, int len)
{
    uint8_t reg, ret;

    reg = addr;
    if(hal_i2c_write(&i2c1, MPL3115A_I2C_ADDRESS, (char const *)&reg, 1, 0) < 0){
        return -1;
    }

    if(hal_i2c_read(&i2c1, MPL3115A_I2C_ADDRESS, (char *)&ret, len, 1) < 0){
        return -1;
    }

    return ret;
}

int mpl3115a2_reset(void)
{
    int16_t sta;

    if(mpl3115a2_write(CTRL_REG1, 0x04)<0){
        //return -1;
    }

    do{
        sta = mpl3115a2_read(CTRL_REG1);
    }while( (sta>=0) && (sta&0x04) != 0 );

    if(sta < 0){
        return -1;
    }
    return 0;
}

int mpl3115a2_init(void)
{
    return 0;
}

uint8_t mpl3115a2_get_id(void)
{
    return mpl3115a2_read(MPL3115_ID);
}

/* 1: altimeter mode, 0: barometer mode. 0 not wait, 1 wait. */
int mpl3115a2_start_measure(uint8_t mode, uint8_t wait)
{
    uint8_t ctrl_reg1 = MPL3115A2_OSR;  // Set default OSR
    int16_t sta;
    int32_t timeout;

    if(mpl3115a2_reset()<0){
        //return -1;
    }

    if(mode){
        /** altimeter */
        ctrl_reg1 |= 0x80;
    }else{
        /** barometer */
        ctrl_reg1 &= ~0x80;
    }

    mpl3115a2_write( CTRL_REG1, ctrl_reg1 );
    mpl3115a2_write( PT_DATA_CFG_REG, 0x07 );       // Enable data flags

    /** Set active mode */
    ctrl_reg1 |= 0x01;
    mpl3115a2_write( CTRL_REG1, ctrl_reg1 );

    /** Set active mode */
    ctrl_reg1 |= 0x02;
    mpl3115a2_write( CTRL_REG1, ctrl_reg1 );

    if(wait == 0){
        return 0;
    }

    timeout = 400;
    do{
        sta = mpl3115a2_read(STATUS_REG);
        DelayMs(1);
        timeout--;
        if(timeout<=0){
            break;
        }
    }while( (sta>=0) && (sta&0x08) == 0 );

    if(sta < 0){
        return -1;
    }

    return 0;
}

int mpl3115a2_check_status(void)
{
    int16_t sta;

    sta = mpl3115a2_read(STATUS_REG);

    if( sta < 0 ){
        return -1;
    }

    if( (sta&0x08) != 0){
        return 0;
    }

    return -1;
}


/** temp = return_value/256, unit: ¡ãC, NOTE return_value is a signed data
 -1 -> invalid */
int16_t mpl3115a2_read_temperature(void)
{
    uint8_t buf[2];
    uint16_t ret;

    buf[0] = mpl3115a2_read(OUT_T_MSB_REG);
    buf[1] = mpl3115a2_read(OUT_T_LSB_REG);
    ret = ((uint16_t)buf[0]<<8) | buf[1];

    return ret;
}

/** pres = return_value/64, unit: Pa
 0 -> invalid */
uint32_t mpl3115a2_read_pressure(void)
{
    uint8_t buf[3];
    uint32_t ret;
    int i;

    buf[0] = mpl3115a2_read(OUT_P_MSB_REG);
    buf[1] = mpl3115a2_read(OUT_P_CSB_REG);
    buf[2] = mpl3115a2_read(OUT_P_LSB_REG);

    for(i=0, ret=0; i<3; i++){
        ret <<= 8;
        ret |= buf[i];
    }
    if(ret == 0){
        ret = 1;
    }

    return ret;
}

/** altitude = return_value/65536, unit: m  */
int32_t mpl3115a2_read_altitude(void)
{
    uint8_t buf[3];
    uint32_t ret;
    int i;

    buf[0] = mpl3115a2_read(OUT_P_MSB_REG);
    buf[1] = mpl3115a2_read(OUT_P_CSB_REG);
    buf[2] = mpl3115a2_read(OUT_P_LSB_REG);

    for(i=0, ret=0; i<3; i++){
        ret |= buf[i];
        ret <<= 8;
    }

    return ret;
}

int16_t int32_to_int16(int32_t val)
{
    int sign = 0;
    uint32_t tmp = val;
    int16_t ret;

    if(val < 0){
        sign = 1;
        tmp = 0 - val;
    }

    ret = tmp >> 16;

    if(sign){
        //minus
        return (0-ret);
    }

    return tmp;
}

/** pres = return_value*4, unit: Pa
 0 -> invalid */
uint16_t mpl3115a2_read_pressure16(void)
{
    uint8_t buf[3];
    uint16_t ret;
    int i;

    buf[0] = mpl3115a2_read(OUT_P_MSB_REG);
    buf[1] = mpl3115a2_read(OUT_P_CSB_REG);
    buf[2] = mpl3115a2_read(OUT_P_LSB_REG);

    for(i=0, ret=0; i<2; i++){
        ret <<= 8;
        ret |= buf[i];
    }

    return ret;
}
