/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __HAL_UART_H
#define __HAL_UART_H

#include "board.h"

#define HAL_UART_ENTER_MUTEX()			__disable_irq( );
#define HAL_UART_EXIT_MUTEX()			__enable_irq( );

typedef enum{
	NONE,
  	EVEN,
	ODD,
}uart_parity_t;

typedef struct{
	uint32_t baud;
	uint8_t word_length;
	uart_parity_t parity;
	uint8_t stop_bits;
}uart_config_t;

typedef uint8_t (*uart_tx_call_back_t) (void);
typedef void (*uart_rx_call_back_t) (uint8_t);

void hal_uart_init(uart_config_t *config, uart_tx_call_back_t tx_hanlder,
				   uart_rx_call_back_t rx_handler);
uint8_t hal_uart_tx_is_empty(void);
void hal_uart_write_tx_reg(uint8_t val);
void hal_uart_deinit(void);
void hal_uart_deinit_wakeup(int timeout);
bool hal_uart_busy(void);

#endif
