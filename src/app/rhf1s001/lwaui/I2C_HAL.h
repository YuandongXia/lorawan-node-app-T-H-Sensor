#ifndef I2C_HAL_H
#define I2C_HAL_H
//==============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT2x Sample Code (V1.2)
// File      :  I2C_HAL.h
// Author    :  MST
// Controller:  NEC V850/SG3 (uPD70F3740)
// Compiler  :  IAR compiler for V850 (3.50A)
// Brief     :  I2C Hardware abstraction layer
//==============================================================================

//---------- Includes ----------------------------------------------------------
#include "board.h"

//---------- Defines -----------------------------------------------------------
//Processor endian system
//#define BIG ENDIAN   //e.g. Motorola (not tested at this time)
#define LITTLE_ENDIAN  //e.g. PIC, 8051, NEC V850

typedef unsigned char   u8t;      ///< range: 0 .. 255
typedef signed char     i8t;      ///< range: -128 .. +127

typedef unsigned short  u16t;     ///< range: 0 .. 65535
typedef signed short    i16t;     ///< range: -32768 .. +32767

typedef unsigned long   u32t;     ///< range: 0 .. 4'294'967'295
typedef signed long     i32t;     ///< range: -2'147'483'648 .. +2'147'483'647

typedef float           ft;       ///< range: +-1.18E-38 .. +-3.39E+38
typedef double          dt;      ///< range:            .. +-1.79E+308

typedef bool            bt;       ///< values: 0, 1 (real bool used)

typedef union {
  u16t u16;               // element specifier for accessing whole u16
  i16t i16;               // element specifier for accessing whole i16
  struct {
    #ifdef LITTLE_ENDIAN  // Byte-order is little endian
    u8t u8L;              // element specifier for accessing low u8
    u8t u8H;              // element specifier for accessing high u8
    #else                 // Byte-order is big endian
    u8t u8H;              // element specifier for accessing low u8
    u8t u8L;              // element specifier for accessing high u8
    #endif
  } s16;                  // element spec. for acc. struct with low or high u8
} nt16;

typedef union {
  u32t u32;               // element specifier for accessing whole u32
  i32t i32;               // element specifier for accessing whole i32
 struct {
    #ifdef LITTLE_ENDIAN  // Byte-order is little endian
    u16t u16L;            // element specifier for accessing low u16
    u16t u16H;            // element specifier for accessing high u16
    #else                 // Byte-order is big endian
    u16t u16H;            // element specifier for accessing low u16
    u16t u16L;            // element specifier for accessing high u16
    #endif
  } s32;                  // element spec. for acc. struct with low or high u16
} nt32;

typedef enum{
  ACK_ERROR                = 0x01,
  TIME_OUT_ERROR           = 0x02,
  CHECKSUM_ERROR           = 0x04,
  UNIT_ERROR               = 0x08
}etError;

#define assert(x)

void DelayMicroSeconds(uint16_t us);


#define     I2C_SCL_H()     do{ \
                                GPIOB->BSRR = GPIO_BSRR_BS_8; \
                                GPIOB->MODER = ( GPIOB->MODER & ~GPIO_MODER_MODE8 ) \
                                     | GPIO_MODER_MODE8_0; \
                            }while(0)
#define     I2C_SCL_L()     do{ \
                                GPIOB->BSRR = GPIO_BSRR_BR_8; \
                                GPIOB->MODER = ( GPIOB->MODER & ~GPIO_MODER_MODE8 ) \
                                     | GPIO_MODER_MODE8_0; \
                            }while(0)
#define     I2C_SCL_INPUT() do{ \
                                GPIOB->MODER = GPIOB->MODER & ~GPIO_MODER_MODE8; \
                            }while(0)
#define     I2C_SCL_IN()    ( ((GPIOB->IDR & GPIO_IDR_ID8) == 0)?LOW:HIGH)

#define     I2C_SDA_H()     do{ \
                                GPIOB->BSRR = GPIO_BSRR_BS_9; \
                                GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE9) \
                                    | GPIO_MODER_MODE9_0; \
                            }while(0)
#define     I2C_SDA_L()     do{ \
                                GPIOB->BSRR = GPIO_BSRR_BR_9; \
                                GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE9) \
                                    | GPIO_MODER_MODE9_0; \
                            }while(0)
#define     I2C_SDA_INPUT() do{ \
                                GPIOB->MODER = GPIOB->MODER & ~GPIO_MODER_MODE9; \
                            }while(0)
#define     I2C_SDA_IN()    ( ((GPIOB->IDR & GPIO_IDR_ID9) == 0)?LOW:HIGH)



//---------- Defines -----------------------------------------------------------
//I2C ports
//The communication on SDA and SCL is done by switching pad direction
//For a low level on SCL or SDA, direction is set to output. For a high level on
//SCL or SDA, direction is set to input. (pull up resistor active)
//#define SDA      PM3H_bit.no0   //SDA on I/O P38 defines direction (input=1/output=0)
//#define SDA_CONF P3H_bit.no0  //SDA level on output direction
//#define SCL      PM3H_bit.no1   //SCL on I/O P39 defines direction (input=1/output=0)
//#define SCL_CONF P3H_bit.no1  //SCL level on output direction


//---------- Enumerations ------------------------------------------------------
//  I2C level
typedef enum{
  LOW                      = 0,
  HIGH                     = 1,
}etI2cLevel;

// I2C acknowledge
typedef enum{
  ACK                      = 0,
  NO_ACK                   = 1,
}etI2cAck;

#define I2C_ADDR_W(addr)            (addr)
#define I2C_ADDR_R(addr)            ((addr) | 0x01)

void I2c_Deinit ();
//==============================================================================
void I2c_Init ();
//==============================================================================
//Initializes the ports for I2C interface

//==============================================================================
void I2c_StartCondition ();
//==============================================================================
// writes a start condition on I2C-bus
// input : -
// output: -
// return: -
// note  : timing (delay) may have to be changed for different microcontroller
//       _____
// SDA:       |_____
//       _______
// SCL :        |___

//==============================================================================
void I2c_StopCondition ();
//==============================================================================
// writes a stop condition on I2C-bus
// input : -
// output: -
// return: -
// note  : timing (delay) may have to be changed for different microcontroller
//              _____
// SDA:   _____|
//            _______
// SCL :  ___|

//===========================================================================
u8t I2c_WriteByte (u8t txByte);
//===========================================================================
// writes a byte to I2C-bus and checks acknowledge
// input:  txByte  transmit byte
// output: -
// return: error
// note: timing (delay) may have to be changed for different microcontroller

//===========================================================================
u8t I2c_ReadByte (etI2cAck ack);
//===========================================================================
// reads a byte on I2C-bus
// input:  rxByte  receive byte
// output: rxByte
// note: timing (delay) may have to be changed for different microcontroller

#endif