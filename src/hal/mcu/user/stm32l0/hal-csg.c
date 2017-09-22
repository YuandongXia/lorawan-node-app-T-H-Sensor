/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "hal-csg.h"
#include "uartd.h"

void hal_csg_init(csg_cb_t cb)
{
    uartd_config_t uart_config;
    uart_config.baud = 2400;
    uart_config.word_length = 8;
    uart_config.parity = EVEN;
    uart_config.stop_bits = 1;
    uartd_init(&uart_config, cb);
}

int hal_csg_rx(uint8_t *buf, int len, uint32_t timeout)
{
    uint32_t cnt;
    int i;
    int16_t ret;

    for(i = 0; i < len; i++){
        cnt = 0;
        while(cnt < timeout){
            cnt++;
            ret = uartd_getchar();
            if(ret >= 0){
                buf[i] = (uint8_t)ret;
                break;
            }
            DelayMs(1);
        }
        if(cnt >= timeout){
            break;
        }
    }
    if( i == len ){
        return 0;
    }
    return -1;
}

void hal_csg_tx(uint8_t *buf, uint8_t len)
{
    uartd_putbuf(buf, len);
}
