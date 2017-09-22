/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __LWLW_H
#define __LWLW_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "timer.h"
#include "LoRaMac.h"

#define LW_PORT                     (8)
#define LW_KEY_LEN                  (16)
#define LW_EUI_LEN                  (8)

typedef struct{
    uint8_t deveui[8];
    uint8_t appeui[8];
    uint32_t devaddr;
    uint32_t netid;
    uint8_t nwkskey[16];
    uint8_t appskey[16];
    uint8_t appkey[16];
    uint8_t port;
}lw_t;

typedef enum{
    LW_MODE_ABP,
    LW_MODE_OTAA,
}lw_mode_t;

typedef struct{
    Mcps_t type;
    uint8_t multicast;
    uint8_t fpending;
    int8_t dr;
    int16_t rssi;
    uint8_t snr;
    uint8_t win;
    uint8_t port;
 //uint8_t buf[256];
    uint8_t *buf;
    uint16_t size;
    uint8_t *maccmd;
    uint8_t maccmd_size;
    uint8_t link_alive;
}lw_rx_info_t;

typedef enum{
    LW_STA_IDLE,
    LW_STA_DONE,
    LW_STA_RX,
    LW_STA_ACK_RX,
    LW_STA_ACK,
    LW_STA_ACK_TIMEOUT,
    LW_STA_JOINED,
    LW_STA_JOIN_FAIL,
    LW_STA_RX_TIMEOUT,
    LW_STA_RX_INVALID,
    LW_STA_CMD_RECEIVED,
    LW_STA_ERROR
}lw_sta_t;

typedef enum{
    LW_JOIN_NORMAL,
    LW_JOIN_FORCE,
}lw_join_t;

//#define LW_DEBUG(x...)          printf(x)

lw_rx_info_t *lw_init(PhyType_t band, lw_mode_t mode);
int lw_join(lw_join_t jtype);
int lw_send(Mcps_t type, uint8_t port, uint8_t *buf, int size, int retry, int8_t dr);
void lw_evt(void);
bool lw_get_evt(lw_sta_t *sta);

#endif
