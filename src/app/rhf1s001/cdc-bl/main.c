/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "clock.h"
#include "uart.h"
#include "uart.h"
#include "hal-tim.h"
#include "ymodem.h"
#include "nvm.h"
#include "hal-led.h"
#include "usb-cdc.h"

uint32_t t = 0;

/* 0: app mode, 1: dfu mode */
uint32_t dfu_check(void)
{
    int i;
    Gpio_t key;

    GpioInit( &key, USB_ON, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );

    for(i=0; i<10000; i++){
        if(GpioRead(&key) == 1){
            return 1;
        }
    }

    return 0;
}

typedef  void ( *pFunction )( void );
pFunction Jump_To_Application;
uint32_t JumpAddress;

#pragma location=(APP_START_ADDR-16)
__root const char bootloader_magic_number[16]="nU-4#e!ruka*edRa";

const char risinghf[5][81] = {
    "    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/",
    "   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/       ",
    "  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/    ",
    " _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/         ",
    "_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/    v0.5.0"
};

void banner(void)
{
    ymodem_putstr("\r\n");
    for(int i=0; i<5; i++){
        ymodem_putstr((char *)risinghf[i]);
        ymodem_putstr("\r\n");
    }
    ymodem_putstr("\r\n");
}

int main(void)
{
    bool sta = false, last_sta = false;

    clock_init(0);

    if( dfu_check() == 0 ){
        /** Check Vector Table (Stack Pointer) */
        if( ( ( *( __IO uint32_t* )APP_START_ADDR ) & 0x2FFE0000 ) == 0x20000000 ){
            /* Jump to user application */
            JumpAddress = *( __IO uint32_t* )( APP_START_ADDR + 4 );
            Jump_To_Application = ( pFunction ) JumpAddress;
            /* Initialize user application's Stack Pointer */
            __set_MSP( *( __IO uint32_t* ) APP_START_ADDR );
            Jump_To_Application( );
        }
    }

    hal_tim_init();

    ym_init();

    t = millis();

    while(1){
        sta = UsbMcuIsDeviceConfigured( );
        if(sta != last_sta){
            last_sta = sta;
            if(sta == true){
                banner();
                ymodem_putstr("Bootloader Mode\r\n");
            }
        }
        ym_event();
        if(millis()-t > 500){
            t = millis();
        }
    }
}