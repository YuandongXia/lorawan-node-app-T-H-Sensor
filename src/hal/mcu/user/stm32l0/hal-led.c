/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "hal-led.h"

Gpio_t led[LED_MAX_NUMS];
uint8_t led_val;

void hal_led_init(void)
{
    GpioInit( &led[0], LED_0, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED0);

#if (LED_MAX_NUMS > 1)
    GpioInit( &led[1], LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED1);
#endif

#if (LED_MAX_NUMS > 2)
    GpioInit( &led[2], LED_2, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED2);
#endif

#if (LED_MAX_NUMS > 3)
    GpioInit( &led[3], LED_3, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED3);
#endif

#if (LED_MAX_NUMS > 4)
    GpioInit( &led[4], LED_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED4);
#endif

#if (LED_MAX_NUMS > 5)
    GpioInit( &led[5], LED_5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    hal_led_off(LED5);
#endif

    led_val = 0;
}

void hal_led_on(led_index_t led_index)
{
    if(led_index>=LED_MAX_NUMS){
        return;
    }
    GpioInit( &led[led_index], led[led_index].pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioWrite( &led[led_index], LED_ON );
    led_val |= (1<<led_index);
}

void hal_led_off(led_index_t led_index)
{
    if(led_index>=LED_MAX_NUMS){
        return;
    }
    GpioWrite( &led[led_index], LED_OFF );
    GpioInit( &led[led_index], led[led_index].pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    led_val &= ~(1<<led_index);
}

void hal_led_toggle(led_index_t led_index)
{
    if(led_index>=LED_MAX_NUMS){
        return;
    }
    if(led_val & (1<<led_index)){
        hal_led_off(led_index);
    }else{
        hal_led_on(led_index);
    }
}

