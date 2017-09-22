/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __STR2HEX_H
#define __STR2HEX_H

#include <stdint.h>

typedef enum{
    STR2HEX_NORMAL,
    STR2HEX_STRICTED,       // raise error when odd length word is received
}str2hex_rule_t;

int str2hex(char *str, uint8_t *hex, str2hex_rule_t rule);

#endif
