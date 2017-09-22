/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __GOLLUM_H
#define __GOLLUM_H

#ifdef USE_USB
#include "usb-cdc.h"
#else
#include "uartd.h"
#endif

#define GM_DEBUG

#define GM_CODE_HEAD                        (0x53)
#define GM_CODE_CMD_ERROR                   (0xFF)

#define GM_OFT_HEAD                         (0)
#define GM_OFT_LEN0                         (1)
#define GM_OFT_LEN1                         (2)
#define GM_OFT_CMD                          (3)

#ifdef USE_USB
#define  gm_putchar(c)                      usb_cdc_putchar(c)
#define  gm_putstr(str)                     usb_cdc_putstring(str)
#define  gm_putbuf(b,l)                     usb_cdc_putbuf(b,l)
#else
#define  gm_putchar(c)                      uartd_putchar(c)
#define  gm_putstr(str)                     uartd_putstring(str)
#define  gm_putbuf(b,l)                     uartd_putbuf(b,l)
#endif

#ifdef GM_DEBUG
#define gm_debug(str)                       gm_putstr(str)
#define gm_debug_puthex(c)                  uart_puthex(c)
#define gm_debug_putbuf_hex(b,l)            uart_putbuf_hex(b,l)
#else
#define gm_debug(str)
#define gm_debug_puthex(c)
#define gm_debug_putbuf_hex(b,l)
#endif

void gm_deinit(void);
void gm_init(void);
void gm_ack(uint8_t cmd, int8_t code);
void gm_tx(uint8_t cmd, int8_t code, uint8_t *buf, int16_t len);
void gm_sta_evt(void);

#endif
