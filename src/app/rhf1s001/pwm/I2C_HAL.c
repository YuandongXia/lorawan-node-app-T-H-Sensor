//==============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT2x Sample Code (V1.2)
// File      :  I2C_HAL.c
// Author    :  MST
// Controller:  NEC V850/SG3 (uPD70F3740)
// Compiler  :  IAR compiler for V850 (3.50A)
// Brief     :  I2C Hardware abstraction layer
//==============================================================================

//---------- Includes ----------------------------------------------------------
#include "I2C_HAL.h"

#define TIM2_CLK                    (1000000)


void i2c_tick_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->CNT = 0;
    TIM2->ARR = 0xFFFF;
    TIM2->PSC = SystemCoreClock/2/TIM2_CLK - 1;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void i2c_tick_deinit(void)
{
    RCC->APB1RSTR |= (RCC_APB1RSTR_TIM2RST);
    RCC->APB1RSTR &= ~ (RCC_APB1RSTR_TIM2RST);
}

void DelayMicroSeconds(uint16_t us)
{
    uint16_t end_value;
    end_value = TIM2->CNT + us*(TIM2_CLK/1000000);
    while(TIM2->CNT < end_value);
}

void I2c_Deinit ()
//==============================================================================
{
    Gpio_t gpio;
    GpioInit( &gpio, I2C_SCL, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &gpio, I2C_SDA, PIN_ANALOGIC, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    i2c_tick_deinit();
}


//==============================================================================
void I2c_Init ()
//==============================================================================
{
    Gpio_t gpio;
    GpioInit( &gpio, I2C_SCL, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioWrite(&gpio, 0);
    GpioInit( &gpio, I2C_SDA, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioWrite(&gpio, 0);
    i2c_tick_init();

    I2C_SDA_L();
    I2C_SCL_L();
    I2C_SDA_H();
    I2C_SCL_H();
}

//==============================================================================
void I2c_StartCondition ()
//==============================================================================
{
    I2C_SDA_H();
    I2C_SCL_H();
    DelayMicroSeconds(1);
    I2C_SDA_L();
    DelayMicroSeconds(1);
    I2C_SCL_L();
    DelayMicroSeconds(1);
}

//==============================================================================
void I2c_StopCondition ()
//==============================================================================
{
    I2C_SCL_L();
    I2C_SDA_L();
    DelayMicroSeconds(1);
    I2C_SCL_H();
    DelayMicroSeconds(1);
    I2C_SDA_H();
    DelayMicroSeconds(1);
}

//==============================================================================
u8t I2c_WriteByte (u8t txByte)
//==============================================================================
{
    u8t mask,error=0;
    for (mask=0x80; mask>0; mask>>=1)   //shift bit for masking (8 times)
    {
        I2C_SCL_L();
        if ((mask & txByte) == 0)
            I2C_SDA_L();//masking txByte, write bit to SDA-Line
        else
            I2C_SDA_H();
        DelayMicroSeconds(1);             //data set-up time (t_SU;DAT)
        I2C_SCL_H();                        //generate clock pulse on SCL
        DelayMicroSeconds(1);             //SCL high time (t_HIGH)
    }
    I2C_SCL_L();
    I2C_SDA_H();
    I2C_SDA_INPUT();                           //release SDA-line
    DelayMicroSeconds(1);
    I2C_SCL_H();                           //clk #9 for ack
    DelayMicroSeconds(1);               //data set-up time (t_SU;DAT)
    if(I2C_SDA_IN()==HIGH)
        error=ACK_ERROR; //check ack from i2c slave
    I2C_SCL_L();
    DelayMicroSeconds(1);              //wait time to see byte package on scope
    return error;                       //return error code
}

//==============================================================================
u8t I2c_ReadByte (etI2cAck ack)
//==============================================================================
{
    u8t mask,rxByte=0;

    I2C_SCL_L();
    I2C_SDA_INPUT();
    for (mask=0x80; mask>0; mask>>=1)   //shift bit for masking (8 times)
    {
        I2C_SCL_L();
        DelayMicroSeconds(1);
        I2C_SCL_H();                         //start clock on SCL-line
        DelayMicroSeconds(1);             //data set-up time (t_SU;DAT)
        if (I2C_SDA_IN()==1)
            rxByte=(rxByte | mask); //read bit
    }

    I2C_SCL_L();
    //send acknowledge if necessary
    if(ack == ACK ){
        I2C_SDA_L();
    }else{
        I2C_SDA_H();
    }
    DelayMicroSeconds(1);             //data hold time(t_HD;DAT)
    I2C_SCL_H();                           //clk #9 for ack
    DelayMicroSeconds(1);               //SCL high time (t_HIGH)

    return rxByte;                      //return error code
}
