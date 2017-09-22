/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "ugpio.h"
#include "rhf76052pins.h"

static PinNames ugpio_tab[32] = {
    PIN0,           // PIN0
    PIN1,           // PIN1
    PIN2,           // PIN2
    PIN3,           // PIN3
    PIN4,           // PIN4
    PIN5,           // PIN5
    PIN6,           // PIN6
    PIN7,           // PIN7
    PIN8,           // PIN8
    PIN9,           // PIN9
    PIN10,          // PIN10
    PIN11,          // PIN11
#ifndef USE_DEBUGGER
    PIN12,          // PIN12
    PIN13,          // PIN13
#else
    NC,
    NC,
#endif
    PIN14,          // PIN14
    PIN15,          // PIN15
    PIN16,          // PIN16
    PIN17,          // PIN17
    PIN18,          // PIN18
    PIN19,          // PIN19
    PIN20,          // PIN20
    PIN21,          // PIN21
    NC, //PIN22,          // PIN22
    NC, //PIN23,          // PIN23
    PIN24,          // PIN24
    PIN25,          // PIN25
    PIN26,          // PIN26
    PIN27,          // PIN27
    PIN28,          // PIN28
    PIN29,          // PIN29
    PIN30,          // PIN30
    PIN31,          // PIN31
};

void ugpio_init(void)
{

}

void ugpio_deinit(void)
{
    Gpio_t gpio;
    int i;

    for(i=0; i<(sizeof(ugpio_tab)/(sizeof(PinNames))); i++){
        if(ugpio_tab[i]==NC){
            continue;
        }
        GpioInit( &gpio, ugpio_tab[i], PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
}

void ugpio_output(uint32_t pin, uint32_t val)
{
    Gpio_t gpio;
    int i;

    for(i=0; i<(sizeof(ugpio_tab)/(sizeof(PinNames))); i++){
        if( 0 == (pin & (1<<i)) ){
            continue;
        }
        if(ugpio_tab[i]==NC){
            continue;
        }
        GpioInit( &gpio, ugpio_tab[i], PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        GpioWrite(&gpio, val&(1<<i));
    }
}


