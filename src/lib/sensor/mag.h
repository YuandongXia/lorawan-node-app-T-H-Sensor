/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __MAG_H
#define __MAG_H

#include <stdbool.h>

#define MAG_IDLE                    (0)
#define MAG_TRIGGERED               (1)
#define MAG_RELEASED                (2)

#define MAG_DOOR_OPENED             (false)
#define MAG_DOOR_CLOSED             (true)

void mag_init();
int mag_get(void);
bool mag_read(void);
bool mag_is_busy();

#endif
