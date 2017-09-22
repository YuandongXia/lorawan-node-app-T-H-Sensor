/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __HAL_UART_DMA_H
#define __HAL_UART_DMA_H

#include "board.h"

#define HAL_UARTD_ENTER_MUTEX()			__disable_irq( );
#define HAL_UARTD_EXIT_MUTEX()			__enable_irq( );

typedef enum{
	NONE,
  	EVEN,
	ODD,
}uartd_parity_t;

typedef struct{
	uint32_t baud;
	uint8_t word_length;
	uartd_parity_t parity;
	uint8_t stop_bits;
}uartd_config_t;

typedef uint8_t (*uart_tx_call_back_t) (void);
typedef void (*uart_rx_call_back_t) (uint8_t);

void hal_uartd_init(uartd_config_t *config, uart_tx_call_back_t tx_hanlder,
				   uart_rx_call_back_t rx_handler);
uint8_t hal_uartd_tx_is_empty(void);
void hal_uartd_write_tx_reg(uint8_t val);
void hal_uartd_deinit(void);
void hal_uartd_deinit_wakeup(int timeout);
bool hal_uartd_busy(void);
bool hal_uartd_rx_busy(void);
#ifndef UART_BL
void hal_uartd_tx_wakeup(void);
#else
#define hal_uartd_tx_wakeup()
#endif

#endif
