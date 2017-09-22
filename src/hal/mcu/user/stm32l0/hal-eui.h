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

Brief: EUI driver

Author: YF
*/

#ifndef __HAL_EUI_H
#define __HAL_EUI_H
#include <stdint.h>

void hal_eui_init(void);
void hal_eui(uint8_t *eui, int len);
int hal_eui64(uint8_t *eui64);

#endif
