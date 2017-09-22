/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/

#ifndef __APP_H
#define __APP_H

#include "sht21.h"

#define LoRaMacAbort()

#ifdef USE_DEBUGGER
#define APP_LINK_CHECK_LIMIT                            (4)
#define APP_LINK_CHECK_LIMIT_MAX                        (24)
#define APP_LINK_CHECK_LIMIT_FAST                       (2)
#else
#define APP_LINK_CHECK_LIMIT                            (64)
#define APP_LINK_CHECK_LIMIT_MAX                        (600)
#define APP_LINK_CHECK_LIMIT_FAST                       (32)
#endif

#define APP_DL_PORT_TIME                                (8)         // 2b
#define APP_DL_PORT_ALARM                               (9)         // 1b

//#define APP_TX_DUTYCYLE_MINUTE                          (2)     // min
//#define APP_TX_DUTYCYLE_MINUTE                          (12345)     // min
//#define APP_TX_DUTYCYCLE_RND                            (1500)   // us
#define APP_DEBUG_DUTYCYCLE                             (6000)   // ms

#define APP_SEND_FAIL_TIMEOUT                           (5000)
#define APP_SEND_SUCCESS_TIMEOUT                        (60000)

void app_init(void);
uint32_t app_get_period(void);
uint32_t app_get_period_oft(void);
int app_set_period(int32_t t);
int app_set_period_oft(int32_t t);

#endif
