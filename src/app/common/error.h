/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __ERROR__H
#define __ERROR__H

#define OK                                  (0)
#define ERR_NONE                            (0)

#define ERR_CRC                             (-1)    // 0xFF
#define ERR_TIMEOUT                         (-2)    // 0xFE
#define ERR_FRAME                           (-3)    // 0xFD
#define ERR_CMD_UNKNOWN                     (-4)    // 0xFC

#define ERR_PARA_INVALID                    (-11)   // 0x
#define ERR_PARA_LEN                        (-12)
#define ERR_API                             (-13)
#define ERR_EEPROM                          (-14)
#define ERR_DISABLED                        (-15)
#define ERR_UNKNOWN                         (-16)
#define ERR_HEAP                            (-17)
#define ERR_UNAVAILABLE                     (-18)

/** WARNINGS */
#define ERR_APP_DR                          (-40)   // printf("+%s: DR%d invalid, set to DR%d\r\n", argv[2], dr, err);


#endif
