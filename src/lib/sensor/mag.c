/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "mag.h"
#include "board.h"

Gpio_t mag;

static TimerEvent_t mag_timeout_timer;
static uint8_t mag_val_old;
bool mag_busy = false;

void mag_irq(void)
{
    TimerStop( &mag_timeout_timer );
    TimerStart( &mag_timeout_timer );

    /* stop interrupt */
    GpioSetInterrupt( &mag, NO_IRQ, IRQ_HIGH_PRIORITY, mag_irq );
    mag_busy = true;
}

void mag_timeout(void)
{
    GpioSetInterrupt( &mag, IRQ_RISING_FALLING_EDGE, IRQ_HIGH_PRIORITY, mag_irq );
    mag_busy = false;
}

void mag_init()
{
    GpioInit( &mag, MAG_INT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioSetInterrupt( &mag, IRQ_RISING_FALLING_EDGE, IRQ_HIGH_PRIORITY, mag_irq );

    TimerInit( &mag_timeout_timer, mag_timeout );
    TimerSetValue( &mag_timeout_timer, 5000000);

    mag_val_old = GpioRead(&mag);
}

/** 0: idle 1: triggered 2: released */
int mag_get(void)
{
    uint8_t mag_val_new;
    uint8_t mag_val;

    mag_val = 0;


    mag_val_new = GpioRead(&mag);
    if( mag_val_old != mag_val_new){
        mag_val_old = mag_val_new;
        TimerStop( &mag_timeout_timer );
        TimerStart( &mag_timeout_timer );
        if(mag_val_new == 0){
            mag_val = 2;
        }else{
            mag_val = 1;
        }
    }

    return mag_val;
}

bool mag_is_busy()
{
    return mag_busy;
}

bool mag_read(void)
{
    if(GpioRead(&mag) == 0){
        return MAG_DOOR_OPENED;
    }else{
        return MAG_DOOR_CLOSED;
    }
}
