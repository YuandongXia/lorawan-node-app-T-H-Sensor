/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "uartd.h"

static volatile uint8_t uart_rx_buf[UART_RX_BUF_LEN]= {0};
static volatile uint8_t uart_tx_buf[UART_TX_BUF_LEN]= {0};
static volatile uint8_t uart_rx_rd_index, uart_rx_cnt, uart_rx_wr_index;
static volatile uint8_t uart_tx_rd_index, uart_tx_cnt, uart_tx_wr_index;
uart_rx_callback_t uart_rx_callback;

uint8_t uart_tx_handler(void)
{
    uint8_t ret;

    if( uart_tx_cnt == 0 ){
        ret = 0;
    }else{
        ret = uart_tx_cnt;

        /** TX buffer is not empty, transmit next byte */
        hal_uartd_write_tx_reg(uart_tx_buf[uart_tx_rd_index++]);
        --uart_tx_cnt;
        if (uart_tx_rd_index == UART_TX_BUF_LEN) {
            uart_tx_rd_index=0;
        }
    }

    return ret;
}

void uart_rx_handler(uint8_t data)
{
    if(uart_rx_cnt == UART_RX_BUF_LEN){
        /** No more space for receive, discard data */
        return;
    }

    if(uart_rx_callback != NULL){
        /** Handle data with uart rx callback, don't save data to buffer. */
        uart_rx_callback(data);
        return;
    }

    if(uart_rx_buf == NULL){
        while(1);
    }

    /** data to buffer */
    uart_rx_buf[uart_rx_wr_index++]=data;
    /** TX write pointer point to head */
    if (uart_rx_wr_index == UART_RX_BUF_LEN) {
        uart_rx_wr_index=0;
    }
    /** remain data in TX buffer plus one */
    ++uart_rx_cnt;
}

void uartd_init(uartd_config_t *config, uart_rx_callback_t cb)
{
    uart_rx_rd_index=0, uart_rx_cnt=0, uart_rx_wr_index=0;
    uart_tx_rd_index=0, uart_tx_cnt=0, uart_tx_wr_index=0;

    uart_rx_callback = cb;
    hal_uartd_init(config, uart_tx_handler, uart_rx_handler);
}

void uartd_deinit(int timeout)
{
    hal_uartd_deinit_wakeup(timeout);
}

//void uartd_disable(void)
//{
//    hal_uartd_deinit();
//}

bool uartd_busy(void)
{
    return hal_uartd_busy();
}

void uartd_putchar(uint8_t c)
{
    /** Reinit TX if it is just wake up */
    hal_uartd_tx_wakeup();

    /** wait until TX buffer is not full */
    while(uart_tx_cnt == UART_TX_BUF_LEN);

    HAL_UARTD_ENTER_MUTEX();
    /** TX buffer is not empty or TX modoule is not busy */
    if(uart_tx_cnt || (hal_uartd_tx_is_empty() == 0)) {
        /** data to buffer */
        uart_tx_buf[uart_tx_wr_index++]=c;
        /** TX write pointer point to head */
        if (uart_tx_wr_index == UART_TX_BUF_LEN) {
            uart_tx_wr_index=0;
        }
        /** remain data in TX buffer plus one */
        ++uart_tx_cnt;
    } else {
        /** TX modoule idle, data to USART data register */
        hal_uartd_write_tx_reg(c);
    }
    HAL_UARTD_EXIT_MUTEX();
}

void uartd_tx_wait(void)
{
    while(uart_tx_cnt>0);
    DelayMs(5);
}

int16_t uartd_getchar(void)
{
    int16_t ret;

    if( uart_rx_cnt == 0 ){
        return -1;
    }

    if( uart_rx_buf == NULL ){
        while(1);
    }

    HAL_UARTD_ENTER_MUTEX();
    ret = uart_rx_buf[uart_rx_rd_index++];
    --uart_rx_cnt;
    if (uart_rx_rd_index == UART_RX_BUF_LEN) {
        uart_rx_rd_index=0;
    }
    HAL_UARTD_EXIT_MUTEX();

    return ret;
}

int uartd_readable()
{
    return uart_rx_cnt;
}

void uartd_putbuf(uint8_t *buf, uint8_t len)
{
    while(len--){
        uartd_putchar(*buf++);
    }
}

void uartd_putstring(char *str)
{
    while(*str){
        uartd_putchar(*str++);
    }
}

static const char hex_tab[] = "0123456789ABCDEF";
void uartd_puthex(uint8_t data)
{
    uint8_t tmp;
    tmp = hex_tab[data>>4];
    uartd_putchar(tmp);
    tmp = hex_tab[data&0x0F];
    uartd_putchar(tmp);
}

void uartd_putbuf_hex(uint8_t *buf, uint8_t len)
{
    uint8_t i;
    for(i=0; i<len-1; i++){
        uartd_puthex(buf[i]);
        uartd_putchar(' ');
    }
    uartd_puthex(buf[i]);
}
