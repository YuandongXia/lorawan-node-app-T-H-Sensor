/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __UART_H
#define __UART_H

#include "uart-board.h"

#define UART_RX_BUF_LEN 				(220)
#define UART_TX_BUF_LEN 				(220)

typedef void (*uart_rx_callback_t)(uint8_t);

void uart_init(uart_config_t *config, uart_rx_callback_t cb);
void uart_deinit(int timeout);
void uart_disable(void);

int uart_readable(void);
//int uart_writable();
void uart_putchar(uint8_t c);
void uart_putstring(char *str);
void uart_putbuf(uint8_t *buf, uint8_t len);
void uart_puthex(uint8_t val);
void uart_putbuf_hex(uint8_t *buf, uint8_t len);
int16_t uart_getchar(void);

void uart_tx_wait(void);
bool uart_busy(void);

extern bool uart_wakeup_flag;

#endif

