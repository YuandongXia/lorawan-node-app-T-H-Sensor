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
#include "tsl2561.h"

static uint8_t CH0_LOW,CH0_HIGH,CH1_LOW,CH1_HIGH;
static uint16_t ch0,ch1;
static unsigned long chScale;
static unsigned long channel1;
static unsigned long channel0;
static unsigned long  ratio1;
static unsigned int b;
static unsigned int m;
static signed long temp;
static unsigned long lux;

int tsl2561_write(uint8_t addr, uint8_t val)
{
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = val;
    if( hal_i2c_write(&i2c1, TSL2561_Address, (char const *)buf, 2, 1) < 0 ){
        return -1;
    }

    return 0;
}

int16_t tsl2561_read(uint8_t addr)
{
    uint8_t reg, ret;

    reg = addr;
    if(hal_i2c_write(&i2c1, TSL2561_Address, (char const *)&reg, 1, 0) < 0){
        return -1;
    }

    if(hal_i2c_read(&i2c1, TSL2561_Address, (char *)&ret, 1, 1) < 0){
        return -1;
    }

    return ret;
}

void tsl2561_init(void)
{
    int ret;

    ret = tsl2561_write(TSL2561_Control,0x03);        // POWER UP
    if(ret<0){
        return;
    }
    ret = tsl2561_write(TSL2561_Timing,0x00);         //No High Gain (1x), integration time of 13ms
    if(ret<0){
        return;
    }
    ret = tsl2561_write(TSL2561_Interrupt,0x00);
    if(ret<0){
        return;
    }
    ret = tsl2561_write(TSL2561_Control,0x00);        // POWER Down
    if(ret<0){
        return;
    }
}

unsigned long tsl2561_calc_lux(unsigned int iGain, unsigned int tInt,int iType)
{
    switch (tInt)
    {
    case 0:  // 13.7 msec
        chScale = CHSCALE_TINT0;
        break;
    case 1: // 101 msec
        chScale = CHSCALE_TINT1;
        break;
    default: // assume no scaling
        chScale = (1 << CH_SCALE);
        break;
    }
    if (!iGain)  chScale = chScale << 4; // scale 1X to 16X
    // scale the channel values
    channel0 = (ch0 * chScale) >> CH_SCALE;
    channel1 = (ch1 * chScale) >> CH_SCALE;

    ratio1 = 0;
    if (channel0!= 0) ratio1 = (channel1 << (RATIO_SCALE+1))/channel0;
    // round the ratio value
    signed long ratio = (ratio1 + 1) >> 1;

    switch (iType)
    {
    case 0: // T package
        if ((ratio >= 0) && (ratio <= K1T))
        {b=B1T; m=M1T;}
        else if (ratio <= K2T)
        {b=B2T; m=M2T;}
        else if (ratio <= K3T)
        {b=B3T; m=M3T;}
        else if (ratio <= K4T)
        {b=B4T; m=M4T;}
        else if (ratio <= K5T)
        {b=B5T; m=M5T;}
        else if (ratio <= K6T)
        {b=B6T; m=M6T;}
        else if (ratio <= K7T)
        {b=B7T; m=M7T;}
        else if (ratio > K8T)
        {b=B8T; m=M8T;}
        break;
    case 1:// CS package
        if ((ratio >= 0) && (ratio <= K1C))
        {b=B1C; m=M1C;}
        else if (ratio <= K2C)
        {b=B2C; m=M2C;}
        else if (ratio <= K3C)
        {b=B3C; m=M3C;}
        else if (ratio <= K4C)
        {b=B4C; m=M4C;}
        else if (ratio <= K5C)
        {b=B5C; m=M5C;}
        else if (ratio <= K6C)
        {b=B6C; m=M6C;}
        else if (ratio <= K7C)
        {b=B7C; m=M7C;}
    }
    temp=((channel0*b)-(channel1*m));
    if(temp<0) temp=0;
    temp+=(1<<(LUX_SCALE-1));
    // strip off fractional portion
    lux=temp>>LUX_SCALE;
    return (lux);
}

int tsl2561_get_lux()
{
    int ret;
    ret = tsl2561_read(TSL2561_Channal0L);
    if(ret<0){
        return -1;
    }
    CH0_LOW = ret;

    ret = tsl2561_read(TSL2561_Channal0H);
    if(ret<0){
        return -1;
    }
    CH0_HIGH = ret;

    //read two bytes from registers 0x0E and 0x0F
    ret = tsl2561_read(TSL2561_Channal1L);
    if(ret<0){
        return -1;
    }
    CH1_LOW = ret;

    ret = tsl2561_read(TSL2561_Channal1H);
    if(ret<0){
        return -1;
    }
    CH1_HIGH = ret;

    ch0 = (CH0_HIGH<<8) | CH0_LOW;
    ch1 = (CH1_HIGH<<8) | CH1_LOW;

    return 0;
}

int32_t tsl2561_read_lux()
{
    // POWER UP
    if(tsl2561_write(TSL2561_Control,0x03) < 0){
        return -1;
    }

    DelayMs(14);

    if( tsl2561_get_lux() < 0){
        return -1;
    }

    // POWER Down
    if(tsl2561_write(TSL2561_Control,0x00) < 0){
        return -1;
    }

    if( ch1 != 0 && ch0/ch1 < 2 && ch0 > 4900 )
    {
        return -1;  //ch0 out of range, but ch1 not. the lux is not valid in this situation.
    }

    return tsl2561_calc_lux(0, 0, 0);  //T package, no gain, 13ms
}

