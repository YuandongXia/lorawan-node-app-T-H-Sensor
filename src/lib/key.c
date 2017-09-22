/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "key.h"
#include "board.h"

Gpio_t key_left;
Gpio_t key_ok;
Gpio_t key_right;

static TimerEvent_t key_timeout_timer;
static TimerEvent_t key_scan_timer;

static uint8_t key_bsy;
static uint8_t key_sta;
static uint8_t key_scan_update;

void key_irq(void)
{
    key_bsy = 1;
    TimerStop( &key_timeout_timer );
    TimerStart( &key_timeout_timer );

    TimerStart( &key_scan_timer );

    /* stop interrupt */
#ifdef KEY_OK
    GpioSetInterrupt( &key_ok, NO_IRQ, IRQ_HIGH_PRIORITY, key_irq );
#endif
    key_scan_update = 0;
    TimerStart( &key_scan_timer );  // continue scan
}

void key_timeout(void)
{
    key_bsy = 0;
#ifdef KEY_OK
    GpioSetInterrupt( &key_ok, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, key_irq );
#endif
    key_scan_update = 0;
    TimerStop( &key_scan_timer );  // continue scan
}

void key_scan(void)
{
    key_scan_update = 1;
}

void key_init()
{
#ifdef KEY_OK
    GpioInit( &key_ok, KEY_OK, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioSetInterrupt( &key_ok, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, key_irq );
#endif
#ifdef KEY_LEFT
    GpioInit( &key_left, KEY_LEFT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
#endif
#ifdef KEY_RIGHT
    GpioInit( &key_right, KEY_RIGHT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
#endif


    TimerInit( &key_timeout_timer, key_timeout );
    TimerSetValue( &key_timeout_timer, 5000);

    key_bsy = 0;
    key_sta = 0;

    key_scan_update = 0;
    TimerInit( &key_scan_timer, key_scan );
    TimerSetValue( &key_scan_timer, 5);
}

int key_busy(void)
{
    return key_bsy;
}

uint8_t key_value(void)
{
    uint8_t key_val, key_cnt;
    key_val = 0;
    key_cnt = 0;
#ifdef KEY_OK
    if( GpioRead(&key_ok) == 0){
        key_val |= KEY_VAL_OK;
        key_cnt++;
    }
#endif
#ifdef KEY_LEFT
    if( GpioRead(&key_left) == 0){
        key_val |= KEY_VAL_LEFT;
        key_cnt++;
    }
#endif
#ifdef KEY_RIGHT
    if( GpioRead(&key_right) == 0){
        key_val |= KEY_VAL_RIGHT;
        key_cnt++;
    }
#endif
    if(key_cnt != 1){
        key_val = 0;
    }

    return key_val;
}

int key_get(void)
{
    static uint8_t key_val_bak;
    uint8_t key_val;

    key_val = 0;

    if(key_scan_update){
        key_scan_update = 0;
        TimerStart( &key_scan_timer );  // continue scan
        switch(key_sta){
        case 0:
            key_val_bak = key_value();
            if(key_val_bak != 0){
                key_sta = 1;
            }
            break;
        case 1:
            key_val = key_value();
            if( key_val != key_val_bak ){
                key_val = 0;
                key_sta = 0;
            }else{
                key_sta = 2;
            }
            break;
        case 2:
            if( key_value() == 0 ){
                key_sta = 0;
            }else{
                /* key is still hold clear timeout */
                TimerStop( &key_timeout_timer );
                TimerStart( &key_timeout_timer );
            }
            break;
        }
    }

    return key_val;
}
