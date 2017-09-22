/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.
*/
#include "sht2x.h"
#include "sht21.h"
#include "board.h"

#define POLYNOMIAL              (0x131)

int8_t sht_err;
int8_t sht_err1;

/*  Save last detected sensor type,
    start to try to read data from this sensor type next time
        this will make the sensor detect efficency*/
sht_t sht = SHTNONE;

int sht21_check_crc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
    uint8_t crc = 0;
    uint8_t byteCtr;
    //calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);

        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    if (crc != checksum){
        sht_err = -1;
        return -1;
    }else{
        return 0;
    }
}

int sht31_check_crc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
    uint8_t crc = 0xFF;
    uint8_t byteCtr;
    //calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);

        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ POLYNOMIAL;
            else
                crc = (crc << 1);
        }
    }
    if (crc != checksum){
        sht_err1 = -1;
        return -1;
    }else{
        return 0;
    }
}

#if 0
/* Temperature = 175.72*ret/2^16-46.85 */
int32_t sht21_read_temp(void)
{
    uint8_t buf[3];
    uint16_t ret;

    /* non-hold Temperature measure */
    buf[0] = SHT21_REG_TNHOLD;
    if( hal_i2c_write(&i2c1, SHT21_I2C_ADDR, (char const *)buf, 1, 1) < 0 ){
        sht_err = -2;
        return -1;
    }

    DelayMs(200);

    if( hal_i2c_read(&i2c1, SHT21_I2C_ADDR, (char *)buf, 3, 1) < 0 ){
        sht_err = -3;
        return -1;
    }

    ret = ((uint16_t)buf[0]<<8) + buf[1];

    if(sht21_check_crc(buf, 2, buf[2]) < 0){
        sht_err = -4;
        return -1;
    }

    return ret&0xFFFC;
}


/* Humidity = 125*ret/2^16 -6 */
int32_t sht21_read_rh(void)
{
    uint8_t buf[3];
    uint16_t ret;

    /* non-hold Humidity measure */
    buf[0] = SHT21_REG_HNHOLD;
    if( hal_i2c_write(&i2c1, SHT21_I2C_ADDR, (char const *)buf, 1, 1) < 0 ){
        sht_err = -5;
        return -1;
    }

    DelayMs(200);

    if( hal_i2c_read(&i2c1, SHT21_I2C_ADDR, (char *)buf, 3, 1) < 0 ){
        sht_err = -6;
        return -1;
    }

    ret = ((uint16_t)buf[0]<<8) + buf[1];

    if(sht21_check_crc(buf, 2, buf[2]) < 0){
        sht_err = -7;
        return -1;
    }

    return ret&0xFFFC;
}
#endif

int sht2x_read_temp_rh(uint16_t *temp,uint16_t * rh)
{
    u8t  error = 0;
    nt16 sRH;                    //variable for raw humidity ticks
    nt16 sT;                     //variable for raw temperature ticks

    *temp = 0xFFFF;
	*rh = 0xFFFF;

    error |= SHT2x_MeasurePoll(HUMIDITY, &sRH);
    error |= SHT2x_MeasurePoll(TEMP, &sT);
    if(error != 0){
        sht_err = error;
        return -1;
    }

    *temp = sT.u16;
    *rh = sRH.u16;

    return 0;
}

//rh:   rh/(2^16-1) * 100;
//temp: temp/(2^16-1) * 175 -45
int sht3x_read_temp_rh(uint16_t *temp,uint16_t * rh)
{
	uint8_t buf[6];
    u8t  i, error=0;    //error variable

	*temp = 0xFFFF;
	*rh = 0xFFFF;

    //-- write I2C sensor address and command --
    I2c_StartCondition();
    error |= I2c_WriteByte (I2C_ADDR_W(SHT3X_I2C_VSS_ADDR)); // I2C Adr
    error |= I2c_WriteByte (SHT3X_NOCLKSTR_LOWREP_MSB);
    error |= I2c_WriteByte (SHT3X_NOCLKSTR_LOWREP_LSB);

    i=0;
    do{
        I2c_StartCondition();
        DelayMs(10);  //delay 10ms
        if(i++ >= 20){
            error |= TIME_OUT_ERROR;
            break;
        }
    }while(I2c_WriteByte (I2C_ADDR_R(SHT3X_I2C_VSS_ADDR)) == ACK_ERROR);

    for(i=0; i<6; i++){
        buf[i] = I2c_ReadByte(ACK);
    }
    I2c_StopCondition();

    if(error != 0){
        sht_err1 = error;
        return -1;
    }

    if(sht31_check_crc(buf, 2, buf[2]) < 0){
		return -1;
	}else{
		*temp = ((uint16_t)buf[0]<<8) + buf[1];
	}

	if(sht31_check_crc(&buf[3], 2, buf[5]) < 0){
		return -1;
	}else{
		*rh = ((uint16_t)buf[3]<<8) + buf[4];
	}

    return 0;

#if 0
    /* non-hold Temperature measure */
    buf[0] = SHT3X_NOCLKSTR_LOWREP_MSB;
	buf[1] = SHT3X_NOCLKSTR_LOWREP_LSB;
    if( hal_i2c_write(&i2c1, SHT3X_I2C_VSS_ADDR,(char const *)buf,2,1) < 0 ){
        return -1;
    }

    DelayMs(20);

    if( hal_i2c_read(&i2c1, SHT3X_I2C_VSS_ADDR, (char *)buf, 6, 1) < 0 ){
        return -1;
    }

	if(sht31_check_crc(buf, 2, buf[2]) < 0){
		return -1;
	}else{
		*temp = ((uint16_t)buf[0]<<8) + buf[1];
	}

	if(sht31_check_crc(&buf[3], 2, buf[5]) < 0){
		return -1;
	}else{
		*rh = ((uint16_t)buf[3]<<8) + buf[4];
	}

	//*temp = ((uint16_t)buf[0]<<8) + buf[1];
	//*rh = ((uint16_t)buf[3]<<8) + buf[4];

    return 0;
#endif
}

sht_t shtxx_read_th(sht_fmt_t fmt, uint16_t *temp, uint16_t *rh)
{
    uint16_t t, h;

    // 0: SHT21, FMT_SHT21 (00); 1: SHT21, FMT_STD (01);
    // 2: SHT3x, FMT_SHT21 (10); 3: SHT3x, FMT_STD (11);
    uint16_t sta = 0;

    sht_err = 0;sht_err1 = 0;

    switch(sht){
    case SHTNONE:
    case SHT2X:
    default:
        sht = SHT2X;
        sht2x_read_temp_rh(&t, &h);
        if(t == 0xFFFF){
            sht = SHT3X;
            sht3x_read_temp_rh(&t, &h);
        }
        break;
    case SHT3X:
        sht = SHT3X;
        sht3x_read_temp_rh(&t, &h);
        if(t == 0xFFFF){
            sht = SHT2X;
            sht2x_read_temp_rh(&t, &h);
        }
        break;
    }
    if( (t == 0xFFFF) || (h == 0xFFFF) ){
        sht = SHTNONE;
        return sht;
    }

    // Standard format
    if((fmt != SHT_FMT_SHT21)&&(fmt != SHT_FMT_DIR)){
        sta |= 0x01;
    }
    if(fmt == SHT_FMT_DIR)
    {
      sta |= 0x04 ;
    }
    
    if(sht == SHT3X){
        sta |= 0x02;
    }

    switch(sta){
    case 0:
        // SHT21, FMT_SHT21
        break;
    case 1:
        // SHT21, FMT_STD
        // T = 175.72*t/2^16-46.85
        // ret = (T + 50) * 400 = 4393 * t / 4096 + 1260
        t = (uint16_t)( (uint32_t)t * 4393 / 4096 + 1260 );
        // H = 125*h/2^16-6
        // ret = H * 2 = 125 * h / 2^15 - 12
        h = (uint16_t)( (uint32_t)h * 125 / 32768 - 12 );
        break;
    case 2:
        //SHT3x, FMT_SHT21
        // SHT21: T = 175.72*t_21/2^16-46.85
        // SHT3x: T = t_3x/(2^16-1)*175-45
        t = (uint16_t)((float)0.9959177688*t + 693.6999772365);
        // SHT21: H = 125*h_21/2^16-6
        // SHT3x: H = h/(2^16-1)*100;
        h = (uint16_t)((float)0.8000122072*h + 3145.728);
        break;
    case 3:
        // SHT3x, FMT_STD
        // T = t/(2^16-1)*175-45
        // ret = (T + 50) * 400 = t * 14000 / 13107 - 2000
        t = (uint16_t)( (uint32_t)t * 14000 / 13107 - 2000 );
        // H = h/(2^16-1)*100;
        // ret = H * 2 = h * 200 / (2^16-1) = 40 * h / 13107
        h = (uint16_t)( (uint32_t)h * 40 / 13107 );
        break;
    case 4:
        //SHT21 SHT_FMT_DIR
        break;
    case 6:
        //SHT3x SHT_FMT_DIR
        break;
    }

    *temp = t;
    *rh = h;

    return sht;
}

sht_t shtxx_read_th_float(float *temp, float *rh)
{
    uint16_t t, h;

    switch(sht){
    case SHTNONE:
    case SHT2X:
    default:
        sht = SHT2X;
        sht2x_read_temp_rh(&t, &h);
        if(t == 0xFFFF){
            sht = SHT3X;
            sht3x_read_temp_rh(&t, &h);
        }
        break;
    case SHT3X:
        sht = SHT3X;
        sht3x_read_temp_rh(&t, &h);
        if(t == 0xFFFF){
            sht = SHT2X;
            sht2x_read_temp_rh(&t, &h);
        }
        break;
    }
    if( (t == 0xFFFF) || (h == 0xFFFF) ){
        sht = SHTNONE;
        return sht;
    }

    switch(sht){
    case SHTNONE:
    case SHT2X:
    default:
        // T = 175.72*t/2^16-46.85
        // H = 125*h/2^16-6
        *temp = 0.0026812744*t-46.85;
        *rh   = 0.0019073486*h-6;
        break;
    case SHT3X:
        // T = t/(2^16-1)*175-45
        // H = h/(2^16-1)*100;
        *temp = 0.0026703288*t-45;
        *rh   = 0.0015259022*h;
        break;
    }
    return sht;
}

uint16_t shtxx_error(void)
{
    return ((uint8_t)sht_err) | (((uint8_t)sht_err1)<<8);
}

