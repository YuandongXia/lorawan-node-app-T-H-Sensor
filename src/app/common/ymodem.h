/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __YMODEM_H
#define __YMODEM_H

#include <stdint.h>

#define YM_SOH  0x01
#define YM_STX  0x02
#define YM_EOT  0x04
#define YM_ACK  0x06
#define YM_NAK  0x15
#define YM_CAN  0x18
#define YM_C    0x43

#define YM_D    0x0D

#define YM_RET_FATAL        -1
#define YM_RET_ACK          0
#define YM_RET_ERROR        1
#define YM_RET_NAK          1
#define YM_RET_ACKC         2
#define YM_RET_START        3
#define YM_RET_ONGOING      5
#define YM_RET_END          4

#define DFU_FLAG                        (0x69)
#define DFU_FLAG_ADDR                   (NVM_START_ADDR + 2044)
#define DFU_FORCE_ADDR                  (NVM_START_ADDR + 2045)
#define DFU_APP_SYNC_ADDR               (NVM_START_ADDR + 2046)
#define DFU_COUNT_ADDR                  (NVM_START_ADDR + 2047)

#ifdef USE_USB_CDC
#define APP_START_ADDR                      (0x08003000)
#define  ymodem_putchar(c)                  usb_cdc_putchar(c)
#define  ymodem_putstr(str)                 usb_cdc_putstring(str)
#else
#define APP_START_ADDR                      (0x08002000)
#define  ymodem_putchar(c)                  uartd_putchar(c)
#define  ymodem_putstr(str)                 uartd_putstring(str)
#endif

#ifdef FLASH_128K
#define APP_END_ADDR                        (0x0801FFFF)
#else
#define APP_END_ADDR                        (0x0800FFFF)
#endif

void ym_init(void);
uint16_t ym_crc(uint8_t *buf, int len);
void ym_event(void);

#endif
