/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

  This program and the accompanying materials are made available under the
  terms of the Eclipse Public License v1.0 which accompanies this
  distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

Brief: timer driver

Author: YF
*/
#ifndef __HAL_TIM_H
#define __HAL_TIM_H
#include <stdint.h>

void hal_tim_init(void);
void hal_tim_deinit(void);
uint32_t hal_tim_get_tick(void);
int hal_tim_check_tick(uint32_t time);
uint32_t millis(void);

#endif
