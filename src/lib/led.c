/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "hal-led.h"
#include "led.h"
#include "board.h"

typedef enum{
    LED_IDLE,
    LED_BLINK,
    LED_RESTART,
}led_sta_t;

static TimerEvent_t led_timer[LED_MAX_NUMS];
static led_sta_t led_sta[LED_MAX_NUMS];
static uint32_t led_blink_value[LED_MAX_NUMS];

void led_cb(led_index_t led)
{
    switch(led_sta[led]){
    case LED_IDLE:
        break;
    case LED_RESTART:
        led_sta[led] = LED_BLINK;
        TimerSetValue(&led_timer[led], led_blink_value[led]);
        TimerStart(&led_timer[led]);
        break;
    case LED_BLINK:
        hal_led_off(led);
        led_sta[led] = LED_IDLE;
        break;
    }
}

void led0_cb(void)
{
    led_cb(LED0);
}

#if (LED_MAX_NUMS > 1)
void led1_cb(void)
{
    led_cb(LED1);
}
#endif

#if (LED_MAX_NUMS > 2)
void led2_cb(void)
{
    led_cb(LED2);
}
#endif

#if (LED_MAX_NUMS > 3)
void led3_cb(void)
{
    led_cb(LED3);
}
#endif

#if (LED_MAX_NUMS > 4)
void led4_cb(void)
{
    led_cb(LED4);
}
#endif

#if (LED_MAX_NUMS > 5)
void led5_cb(void)
{
    led_cb(LED5);
}
#endif

 void (* const led_cb_buf[LED_MAX_NUMS])(void) = {
    led0_cb,
#if (LED_MAX_NUMS > 1)
    led1_cb,
#endif
#if (LED_MAX_NUMS > 2)
    led2_cb,
#endif
#if (LED_MAX_NUMS > 3)
    led3_cb,
#endif
#if (LED_MAX_NUMS > 4)
    led4_cb,
#endif
#if (LED_MAX_NUMS > 5)
    led5_cb,
#endif
};

void led_init(void)
{
    int i;

    hal_led_init();

    for(i=0; i<LED_MAX_NUMS; i++){
        TimerInit(&led_timer[i], led_cb_buf[i]);
    }
}

void led_on(led_index_t led_index)
{
    hal_led_on(led_index);
}

void led_off(led_index_t led_index)
{
    hal_led_off(led_index);
}

void led_toggle(led_index_t led_index)
{
    hal_led_toggle(led_index);
}

void led_blink(led_index_t led_index, uint32_t light_on_time)
{
    if(led_index >= LED_MAX_NUMS){
        return;
    }
    switch(led_sta[led_index]){
    case LED_IDLE:
        hal_led_on(led_index);
        led_sta[led_index] = LED_BLINK;
        TimerSetValue(&led_timer[led_index], light_on_time);
        TimerStart(&led_timer[led_index]);
        break;
    case LED_RESTART:
        led_blink_value[led_index] = light_on_time;
        break;
    case LED_BLINK:
        led_blink_value[led_index] = light_on_time;
        led_sta[led_index] = LED_RESTART;
        break;
    }
}

