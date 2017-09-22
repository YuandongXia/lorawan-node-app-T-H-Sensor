/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __HAL_CHINA_STATE_GRID_H
#define __HAL_CHINA_STATE_GRID_H

#include <stdint.h>

typedef void (*csg_cb_t)(uint8_t);

void hal_csg_init(csg_cb_t cb);
int hal_csg_rx(uint8_t *buf, int len, uint32_t timeout);
void hal_csg_tx(uint8_t *buf, uint8_t len);

#endif
