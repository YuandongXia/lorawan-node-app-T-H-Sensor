/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __LORAMAC_BOARD_H__
#define __LORAMAC_BOARD_H__

/*!
 * Returns individual channel mask
 *
 * \param[IN] channelIndex Channel index 1 based
 * \retval channelMask
 */
#define LC( channelIndex )            ( uint16_t )( 1 << ( channelIndex - 1 ) )

/*!
 * LoRaMac datarates definition
 */
#define DR_0                                        0
#define DR_1                                        1
#define DR_2                                        2
#define DR_3                                        3
#define DR_4                                        4
#define DR_5                                        5
#define DR_6                                        6
#define DR_7                                        7
#define DR_8                                        8
#define DR_9                                        9
#define DR_10                                       10
#define DR_11                                       11
#define DR_12                                       12
#define DR_13                                       13
#define DR_14                                       14
#define DR_15                                       15

#endif // __LORAMAC_BOARD_H__
