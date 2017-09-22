/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __CMD_H
#define __CMD_H

#include <stdint.h>
#include "gollum.h"
#include "lw.h"
#include "LoRaMac.h"

#define CMD_OFT_CMD                     (0)
#define CMD_OFT_TYPE                    (1)

#define CMD_ERROR                       (0xFF)
#define CMD_OK                          (0)
#define CMD_MIN_LEN                     (2)

#define CMD_VER                         (0x01)
//#define CMD_RESET                       (0x02)
#define CMD_ID                          (0x03)
#define CMD_ADR                         (0x04)
#define CMD_DR                          (0x05)
#define CMD_REPT                        (0x06)
#define CMD_POWER                       (0x07)
#define CMD_RXWIN2                      (0x08)
#define CMD_KEY                         (0x09)
//#define CMD_DFU                         (0x0A)
#define CMD_MODE                        (0x0B)
#define CMD_CLASS                       (0x0C)
#define CMD_PORT                        (0x0D)
#define CMD_CH                          (0x0E)      // #
#define CMD_FDEFAULT                    (0x0F)
#define CMD_PERIOD                      (0x10)
#define CMD_TEST                        (0x11)
#define CMD_GPIO                        (0x12)
#define CMD_SLEEP                       (0x13)
#define CMD_THRESHOLD                   (0x14)
#define CMD_RXWIN1                      (0x15)
#define CMD_ULFMT                       (0x16)
//#define CMD_METER_FACTOR                (0x17)
//#define CMD_ME1_INIT_PULSE              (0x18)
//#define CMD_ME2_INIT_PULSE              (0x19)
#define CMD_GPS                         (0x20)
#define CMD_MC                          (0x21)

#define CMD_ID_SUB_CHK                  (1)
#define CMD_ID_SUB_SET_DEVADDR          (2)
#define CMD_ID_SUB_SET_DEVEUI           (3)
#define CMD_ID_SUB_SET_APPEUI           (4)
#define CMD_ID_LEN_CHK                  (0)
#define CMD_ID_LEN_SET_DEVADDR          (4)
#define CMD_ID_LEN_SET_DEVEUI           (8)
#define CMD_ID_LEN_SET_APPEUI           (8)

#define CMD_FDEFAULT_LEN                (9)

#define CMD_ADR_LEN                     (2)

#define CMD_THRESHOLD_LEN               (3)

#define CMD_ULFMT_LEN                   (2)

#define CMD_DR_SUB_CHECK                (1)
#define CMD_DR_SUB_DR                   (2)
#define CMD_DR_SUB_BAND                 (3)
#define CMD_DR_LEN_CHECK                (0)
#define CMD_DR_LEN_DR                   (1)
#define CMD_DR_LEN_BAND                 (1)

#define CMD_KEY_SUB_NWKSKEY             (1)
#define CMD_KEY_SUB_APPSKEY             (2)
#define CMD_KEY_SUB_APPKEY              (3)
#define CMD_KEY_LEN_NWKSKEY             (LW_KEY_LEN)
#define CMD_KEY_LEN_APPSKEY             (LW_KEY_LEN)
#define CMD_KEY_LEN_APPKEY              (LW_KEY_LEN)

#define CMD_PERIOD_SUB_PER              (1)
#define CMD_PERIOD_SUB_OFT              (2)
#define CMD_PERIOD_LEN_PER              (4)
#define CMD_PERIOD_LEN_OFT              (4)

#define CMD_CH_LEN_FREQ_DR              (5)

#define CMD_RXWIN1_LEN_FREQ             (4)

#define CMD_GPIO_SUB_DEINIT             (1)
#define CMD_GPIO_SUB_ALL                (2)
#define CMD_GPIO_SUB_EACH               (3)
#define CMD_GPIO_LEN_DEINIT             (0)
#define CMD_GPIO_LEN_ALL                (4)
#define CMD_GPIO_LEN_EACH               (8)

#define CMD_TEST_SUB_STOP               (1)
#define CMD_TEST_SUB_TXCW               (2)
#define CMD_TEST_SUB_TXCLR              (3)
#define CMD_TEST_SUB_RFCFG              (4)
#define CMD_TEST_SUB_TXPKT              (5)
#define CMD_TEST_SUB_RXPKT              (6)
#define CMD_TEST_SUB_RSSI               (7)
#define CMD_TEST_SUB_LWDL               (8)
#define CMD_TEST_SUB_TEST               (9)
#define CMD_TEST_LEN_STOP               (0)
#define CMD_TEST_LEN_TXCW               (0)
#define CMD_TEST_LEN_TXCLR              (0)
#define CMD_TEST_LEN_RFCFG              (9)
#define CMD_TEST_LEN_TXPKT_MIN          (1)
#define CMD_TEST_LEN_RXPKT              (0)
#define CMD_TEST_LEN_RSSI               (8)
#define CMD_TEST_LEN_LWDL               (0)
#define CMD_TEST_LEN_TEST               (1)

typedef enum{
    MSG_TYPE_AT = 0,
}cmd_msg_type_t;

typedef int (*p_cmd_func) (uint8_t *, int);

typedef struct {
    uint8_t cmd;
    p_cmd_func func;
}cmd_list_t;

void cmd_init(void);
int cmd_exec(uint8_t *, int);

extern MulticastParams_t cmd_mc_list;

#endif
