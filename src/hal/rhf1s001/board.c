/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

Description: loramac-node board level functions

*/
#include "board.h"

static void BoardUnusedIoInit( void );

static bool McuInitialized = false;
static bool TimerCalibrated = false;
static TimerEvent_t TimerCalibrationTimer;

Gpio_t dcdcen;

static void OnTimerCalibrationTimerEvent( void )
{
    TimerCalibrated = true;
}

static PinNames McuUnusedIo[] = {
    PIN3,
    PIN4,
    PIN5,
    PIN6,
    PIN7,
    PIN8,
    PIN9,
    PIN10,
    PIN11,

    PIN14,
    PIN15,
    PIN16,
    PIN20,
    PIN21,
    PIN22,
    PIN23,
    PIN24,
    PIN25,
    PIN26,

    PIN28,
};

static void CalibrateTimer( void )
{
    if( TimerCalibrated == false )
    {
        TimerInit( &TimerCalibrationTimer, OnTimerCalibrationTimerEvent );
        TimerSetValue( &TimerCalibrationTimer, 1000 );
        TimerStart( &TimerCalibrationTimer );
        while( TimerCalibrated == false )
        {
            TimerLowPowerHandler( );
        }
    }
}

void BoardInitPeriph( void )
{

}

static void BoardUnusedIoInit( void )
{
    Gpio_t gpio;
    int i;

    for(i=0; i<(sizeof(McuUnusedIo)/(sizeof(PinNames))); i++){
        GpioInit( &gpio, McuUnusedIo[i], PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }

#ifndef USE_DEBUGGER
    Gpio_t dbggpio;
    GpioInit( &dbggpio, SWDIO, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &dbggpio, SWCLK, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif

    GpioInit( &gpio, PIN16, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &gpio, PIN14, PIN_OUTPUT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
}

void BoardInitMcu( void )
{
    if( McuInitialized == false ){
        clock_init(0);

#if defined( USE_BOOTLOADER )
#if defined( BOOTLOADER_12K )
        // Set the Vector Table base location at 0x3000
        SCB->VTOR = FLASH_BASE | 0x3000; /* Vector Table Relocation in Internal FLASH */
#elif defined( BOOTLOADER_8K )
        // Set the Vector Table base location at 0x2400
        SCB->VTOR = FLASH_BASE | 0x2000; /* Vector Table Relocation in Internal FLASH */
#else
#warning "Pleaes define bootloader size"
#endif
#endif

        // Disable Systick
        SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off
        SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag

        RtcInit( );
        BoardUnusedIoInit( );

#ifdef USE_DEBUGGER
        /* Enable debug under stop mode */
        RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;
        DBGMCU->CR |= DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY | DBGMCU_CR_DBG_SLEEP;
        DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_RTC_STOP;
#endif

        McuInitialized = true;
        CalibrateTimer( );

#ifndef USE_DEBUGGER
        RCC->APB2RSTR = RCC_APB2RSTR_DBGMCURST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_DBGMCURST;
#endif
    }else{
        /** clock reconfigure */
        clock_reinit();
    }

    GpioInit( &dcdcen, DCDCEN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioWrite( &dcdcen, DCDCEN_ON );

    //hal_i2c_init(&i2c1, I2C_SCL, I2C_SDA);
   // hal_i2c_frequency(&i2c1, 400000);
    //hal_i2c_frequency(&i2c1, 100000);
    I2c_Init();

    SpiInit( &SX1276.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    SX1276IoInit( );
}

void BoardDeInitMcu( void )
{
    SpiDeInit( &SX1276.Spi );
    SX1276IoDeInit( );

    //hal_i2c_deinit(&i2c1, I2C_SCL, I2C_SDA);
    I2c_Deinit();

    hal_adc_deinit();

    GpioInit( &dcdcen, DCDCEN, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void BoardGetUniqueId( uint8_t *id )
{
    id[0] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[1] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[2] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[3] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[4] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[5] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[6] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[7] = ( ( *( uint32_t* )ID2 ) );
}

uint8_t GetBoardPowerSource( void )
{
    return BATTERY_POWER;
}

uint8_t BoardGetBatteryLevel( void )
{
    return 254;
}
