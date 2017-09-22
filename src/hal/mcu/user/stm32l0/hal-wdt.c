/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"

#define IWDG_ENABLE             ((uint32_t)0xCCCC)
#define IWDG_ACCESS_REG         ((uint32_t)0x5555)
#define IWDG_REFRESH_COUNTER    ((uint32_t)0xAAAA)
#define IWDG_PRESCALER          (0x6)
#define IWDG_UPDATE             (0x00000000)

void IWDG_init(void)
{
    IWDG->KR = IWDG_ENABLE;
    IWDG->KR = IWDG_ACCESS_REG;
    IWDG->PR = IWDG_PRESCALER;
    //IWDG->RLR = 2890;//10s
    IWDG->RLR = 0xFFF;//Actually 30s
    //IWDG->RLR = 3906;//16s
    //IWDG->RLR = 4090;//16s
    //IWDG->RLR = 1000;//30s
    //IWDG->RLR = 3125;//5s
    while (IWDG->SR != IWDG_UPDATE);
}

void IWDG_feed(void)
{
    IWDG->KR = IWDG_REFRESH_COUNTER;
}


//void hal_wdt_init(uint32_t ms)
//{
//    uint32_t tmp, reg;
//    int i;
//
//    if(ms > WDT_MAX_TIMEOUT){
//        ms = WDT_MAX_TIMEOUT;
//    }
//    tmp = ms*(LSI_VALUE/1000);
//
//    IWDG->KR = IWDG_KEY_ENABLE;
//    IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE;
//
//    for(i=0; i<7; i++){
//        if( ( tmp>>(i+2) ) <= 0xFFF ){
//            reg = ( tmp>>(i+2) );
//            break;
//        }
//    }
//    if(i==7){
//        reg = 0xFFF;
//    }
//    IWDG->PR = i;
//    IWDG->RLR = reg;
//    while(IWDG->SR);
//    IWDG->KR = IWDG_KEY_RELOAD;
//}
//
//void hal_wdt_clear(void)
//{
//    IWDG->KR = IWDG_KEY_RELOAD;
//}

