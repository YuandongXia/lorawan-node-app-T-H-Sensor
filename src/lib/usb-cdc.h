/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#include "usb-cdc-board.h"

#define HAL_USB_CDC_ENTER_MUTEX()			__disable_irq( );
#define HAL_USB_CDC_EXIT_MUTEX()			__enable_irq( );

#define USB_CDC_RX_BUF_LEN 				(220)
#define USB_CDC_TX_BUF_LEN 				(220)
#define USB_CDC_TMP_BUF_LEN 			(220)

typedef void (*usb_cdc_rx_callback_t)(uint8_t);

void usb_cdc_init(usb_cdc_rx_callback_t cb);
int usb_cdc_readable(void);
//int usb_cdc_writable();
void usb_cdc_putchar(uint8_t c);
void usb_cdc_putstring(char *str);
void usb_cdc_putbuf(uint8_t *buf, uint8_t len);
void usb_cdc_puthex(uint8_t val);
void usb_cdc_putbuf_hex(uint8_t *buf, uint16_t len);
int16_t usb_cdc_getchar(void);

void usb_cdc_sta(void);

#endif

