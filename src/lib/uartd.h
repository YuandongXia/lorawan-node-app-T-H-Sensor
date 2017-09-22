/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __UARTD_H
#define __UARTD_H

#include "hal-uartdma.h"

#define UART_RX_BUF_LEN 				(220)
#define UART_TX_BUF_LEN 				(220)

typedef void (*uart_rx_callback_t)(uint8_t);

void uartd_init(uartd_config_t *config, uart_rx_callback_t cb);
void uartd_deinit(int timeout);
//void uartd_disable(void);

int uartd_readable(void);
//int uart_writable();
void uartd_putchar(uint8_t c);
void uartd_putstring(char *str);
void uartd_putbuf(uint8_t *buf, uint8_t len);
void uartd_puthex(uint8_t val);
void uartd_putbuf_hex(uint8_t *buf, uint8_t len);
int16_t uartd_getchar(void);

void uartd_tx_wait(void);
bool uartd_busy(void);

extern bool uart_wakeup_flag;
extern bool uart_idle;
extern bool uart_auto_sleep;
extern TimerTime_t uart_tick;

#endif

