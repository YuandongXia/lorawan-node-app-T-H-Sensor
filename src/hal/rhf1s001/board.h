/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: loramac-node board dependent definitions

*/
#ifndef __BOARD_H__
#define __BOARD_H__

/** Standard C library */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/** STML0xx library */
#include "stm32l0xx.h"

/** LoRaWAN node library */
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "spi.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "rtc-board.h"
#include "sx1276-board.h"

/** HAL common library */
#include "clock.h"
#include "adc-board.h"
#include "error.h"

#include "rhf76052pins.h"

#include "I2C_HAL.h"

/*!
 * NULL definition
 */
#ifndef NULL
    #define NULL                                    ( ( void * )0 )
#endif

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 PB_11

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PA_4

#define RADIO_DIO_0                                 PB_10
#define RADIO_DIO_1                                 PB_2
#define RADIO_DIO_2                                 PB_0
#define RADIO_DIO_3                                 PB_1
#define RADIO_DIO_4                                 NC
#define RADIO_DIO_5                                 NC

#define RADIO_ANT_SWITCH_RXTX1	                    PA_1
#define RADIO_ANT_SWITCH_RXTX2                      PA_2

#define I2C_SCL                                     PB_8
#define I2C_SDA                                     PB_9

#ifdef UART_TXPB6_RXPB7
#define UART_TX                                     PB_6
#define UART_RX                                     PB_7
#elif defined(UART_TXPA9_RXPA10)
#define UART_TX                                     PA_9
#define UART_RX                                     PA_10
#endif

#define USB_DM                                      PA_11
#define USB_DP                                      PA_12
#define USB_ON                                      PB_5

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14

#define DCDCEN                                      PC_13
#define DCDCEN_ON                                   (1)
#define DCDCEN_OFF                                  (0)

#define LED_MAX_NUMS                                (2)
#define LED_OFF										(1)
#define LED_ON										(0)
#define LED_0                                       PIN23   // GREEN
#define LED_1                                       PIN22   // RED
#define RF_RX_LED                                   LED1
#define RF_TX_LED                                   LED1
#define USB_RX_LED                                  LED0
#define USB_TX_LED                                  LED0

#define CHECK_POWER_RANGE
#define LF_MAX_POWER                                (20)
#define HF_MAX_POWER                                (14)

enum BoardPowerSource
{
    USB_POWER = 0,
    BATTERY_POWER
};

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source ( 0: USB_POWER,  1: BATTERY_POWER )
 */
uint8_t GetBoardPowerSource( void );


#endif // __BOARD_H__
