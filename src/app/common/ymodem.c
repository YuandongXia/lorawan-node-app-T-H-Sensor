/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "ymodem.h"
#include "board.h"
#include "uartd.h"
#include "hal-tim.h"
#include "flash.h"
#include "aes.h"
#include "nvm.h"
#include "usb-cdc.h"

#define YM_BUF_SIZE             (1029)

typedef enum{
    YM_STA_IDLE,
    YM_STA_WAIT,
    YM_STA_RECEIVING,
    YM_STA_RX_DONE,
    YM_STA_FINISH,
    YM_STA_FATAL,
}ym_sta_t;

const uint8_t key[]={
    0x15, 0xF9, 0xC1, 0x11, 0x17, 0xA2, 0x1F, 0x63,
    0x24, 0x58, 0x58, 0x77, 0xE1, 0x15, 0x16, 0x28,
};

ym_sta_t ym_sta;
uint8_t ym_buf[YM_BUF_SIZE];
uint8_t ym_aligned_buf[1024];
uint8_t ym_dbuf[1024];
int ym_wr_index;
int ym_len;
uint32_t ym_ts;
uint8_t ym_eot_flag;
uint8_t ym_can_flag;
int ym_cnt = 0;
uint8_t ym_pkt_last = 0;
int8_t ym_tx = 0;
bool ym_ts_clear = false;
int ym_retry_cnt = 0;

int encrypt(uint8_t *out, uint8_t *in, uint8_t *key)
{
    aes_context aesContext;
    memset(&aesContext, 0, sizeof(aesContext));
	aes_set_key(key, 16, &aesContext);
    aes_encrypt(in, out, &aesContext);
    return 0;
}

int block_decrypt(uint8_t *out, uint8_t *in, int len, uint8_t *key)
{
    int i;

    for(i=0; i<len; i+=16){
        encrypt(out+i, in+i, key);
    }
    return i;
}

void ym_modem(uint8_t dat)
{
    switch(ym_sta){
    case YM_STA_IDLE:
    case YM_STA_WAIT:
        ym_ts_clear = true;
        ym_buf[0] = dat;
        ym_wr_index = 1;
        switch(ym_buf[0]){
        case YM_SOH:
        case YM_STX:
            ym_sta = YM_STA_RECEIVING;
            break;
        case YM_EOT:
        case YM_CAN:
            ym_len = ym_wr_index;
            ym_wr_index = 0;
            ym_sta = YM_STA_RX_DONE;
            break;
        }
        break;
    case YM_STA_RECEIVING:
        ym_ts_clear = true;
        ym_buf[ym_wr_index++] = dat;
        if( (ym_buf[0] == YM_SOH) && (ym_wr_index == (128+5)) ){
            ym_len = ym_wr_index;
            ym_wr_index = 0;
            ym_sta = YM_STA_RX_DONE;
        }else if( (ym_buf[0] == YM_STX) && (ym_wr_index == (1024+5)) ){
            ym_len = ym_wr_index;
            ym_wr_index = 0;
            ym_sta = YM_STA_RX_DONE;
        }else if(ym_wr_index == YM_BUF_SIZE){
            ym_len = ym_wr_index;
            ym_wr_index = 0;
            ym_sta = YM_STA_RX_DONE;
        }
        break;
    case YM_STA_RX_DONE:

        break;
    }
}

uint16_t ym_crc(uint8_t *buf, int len)
{
    uint16_t crc = 0;
    uint16_t i = 0;

    while (len--){
        crc = crc^(int)(*buf++) << 8;
        for (i=8; i!=0; i--){
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
    }
    return crc;
}

void ym_init(void)
{
#ifdef USE_USB_CDC
    UsbMcuInit();
    usb_cdc_init(ym_modem);
#else
    uartd_config_t uart_config;
    uart_config.baud = 115200;
    uart_config.word_length = 8;
    uart_config.parity = NONE;
    uart_config.stop_bits = 1;
    uartd_init(&uart_config, ym_modem);
#endif

    ym_sta = YM_STA_IDLE;
    ym_wr_index = 0;
    ym_cnt = 0;
    ym_len = 0;
    ym_eot_flag = 0;
    ym_pkt_last = 0;
    ym_tx = 0;
    ym_retry_cnt = 0;
}

void dfu_sync(void)
{
    uint8_t dfu_app_sync, dfu_count;

    /* set dfu_app_sync flag */
    dfu_app_sync = nvm_read_byte(DFU_APP_SYNC_ADDR);
    if(dfu_app_sync != 0){
        nvm_write_byte(DFU_APP_SYNC_ADDR, 0);
    }

    dfu_count = nvm_read_byte(DFU_COUNT_ADDR);
    if( dfu_count != 0){
        nvm_write_byte(DFU_COUNT_ADDR, 0);
    }
}

void ym_restart(void)
{
    ym_sta = YM_STA_IDLE;
    ym_wr_index = 0;
    ym_cnt = 0;
    ym_len = 0;
    ym_eot_flag = 0;
    ym_pkt_last = 0;
    ym_tx = 0;
    ym_retry_cnt = 0;

    ym_ts = millis();
}

void ym_parse(uint8_t *buf, int len)
{
    uint16_t crc_cal;
    uint16_t crc;
    int i;
    uint32_t addr;

    //ym_debug(buf, len);
    ym_sta = YM_STA_WAIT;

    switch(buf[0]){
    case YM_SOH:
    case YM_STX:
        if( (buf[1] ^ buf[2]) != 0xFF){
            ymodem_putchar(YM_NAK);
            ym_tx = YM_RET_NAK;
            break;
        }

        if(len < 5){
            ymodem_putchar(YM_NAK);
            ym_tx = YM_RET_NAK;
            break;
        }

        __disable_irq();
        crc_cal = ym_crc(buf+3, len-5);
        crc = (buf[len-2]<<8) | buf[len-1];
        __enable_irq();


        if(crc_cal != crc){
            ymodem_putchar(YM_NAK);
            ym_tx = YM_RET_NAK;
            break;
        }

        if( (ym_cnt == 0) && (buf[1] == 0) ){
            ym_pkt_last = 0;
            ymodem_putchar(YM_ACK);
            ymodem_putchar(YM_C);
            ym_tx = YM_RET_ACKC;
        }else{
            /** clear eot flag */
            if(ym_eot_flag == 2){
                for(i=0; i<len-5; i++){
                    if(buf[i+3] != 0){
                        ym_sta = YM_STA_FATAL;
                        break;
                    }
                }
                if(i==(len-5)){
                    ym_sta = YM_STA_FINISH;
                    ymodem_putchar(YM_ACK);
                    ym_tx = YM_RET_ACK;
                }
            }else{
                if( buf[1] == (uint8_t)(ym_pkt_last+1) ){
                    i=0;
                    if(ym_cnt == 0){
                        dfu_sync();
                        __disable_irq();
                        flash_unlock();
                        addr = APP_START_ADDR;
                        for(;addr<APP_END_ADDR;addr+=128){
                            flash_erase_page(addr);
                        }
                        __enable_irq();
                    }

                    addr = APP_START_ADDR+ym_cnt;
                    /** force aligned */
                    __disable_irq();
                    memcpy(ym_aligned_buf, buf+3, len-5);
                    block_decrypt(ym_dbuf, ym_aligned_buf, len-5, (uint8_t *)key);
                    for(;(i<len-5) && addr<APP_END_ADDR;addr+=128, i+=128){
                        flash_program_page(addr, 128, ym_dbuf+i);
                    }
                    ym_cnt += len-5;
                    ym_pkt_last = buf[1];
                    __enable_irq();

                    ymodem_putchar(YM_ACK);
                    ym_tx = YM_RET_ACK;
                }else{
                    ym_sta = YM_STA_FATAL;
                    ymodem_putchar(YM_CAN);
                }
            }
        }
        break;
    case YM_EOT:
        if((len == 1) && (ym_eot_flag == 0)){
            /** NACK here */
            ym_eot_flag = 1;
            ymodem_putchar(YM_NAK);
            ym_tx = YM_RET_NAK;
        }else if( (len == 1) && (ym_eot_flag == 1) ){
            ym_eot_flag = 2;
            /** ACK here */
            ymodem_putchar(YM_ACK);
            ymodem_putchar(YM_C);
            ym_tx = YM_RET_ACKC;
        }
        break;
    case YM_CAN:
        if((len == 1) && (ym_can_flag == 0)){
            /** NACK here */
            ym_can_flag = 1;
            ymodem_putchar(YM_NAK);
            ym_tx = YM_RET_NAK;
        }else if( (len == 1) && (ym_can_flag == 1) ){
            ym_can_flag = 2;
            /** ACK here */
            ymodem_putchar(YM_ACK);
            ymodem_putchar(YM_C);
            ym_tx = YM_RET_ACKC;
        }
        ym_sta = YM_STA_FATAL;
        break;
    case YM_D:
        //ym_sta = YM_STA_FINISH;
        ym_restart();
        ymodem_putchar(YM_ACK);
        ym_tx = YM_RET_ACK;
        break;
    default:

        break;
    }
}

void ym_retry(void)
{
    switch(ym_tx){
    case YM_RET_ACKC:
        ymodem_putchar(YM_ACK);
        ymodem_putchar(YM_C);
        break;
    case YM_RET_ACK:
        ymodem_putchar(YM_ACK);
        break;
    case YM_RET_NAK:
        ymodem_putchar(YM_NAK);
        break;
    }
}

void ym_wait(uint32_t ms)
{
    uint32_t t;
    t = millis();
    while( (millis() - t) < ms );
}

void ym_event(void)
{
    switch(ym_sta){
    case YM_STA_IDLE:
        if( (millis()-ym_ts)>1000){
            ym_ts = millis();
            ymodem_putchar('C');
        }
        break;
    case YM_STA_RECEIVING:
        if(ym_ts_clear){
            ym_ts_clear = false;
            ym_ts = millis();
        }
        if( (millis()-ym_ts)>300){
            /** RECEIVING TIMEOUT */
            ym_ts = millis();
            ym_len = ym_wr_index;
            ym_wr_index = 0;
            ym_sta = YM_STA_RX_DONE;
        }
        break;
    case YM_STA_RX_DONE:
        ym_retry_cnt=0;
        ym_parse(ym_buf, ym_len);
        ym_ts = millis();
        break;
    case YM_STA_WAIT:
        if(ym_ts_clear){
            ym_ts_clear = false;
            ym_ts = millis();
        }
        if( (millis()-ym_ts)>200){
            /** WAIT TIMEOUT */
            ym_ts = millis();
            if(ym_wr_index != 0){
                if( (ym_wr_index == 1) || (ym_wr_index == 133) || (ym_wr_index == 1029) ){
                    ym_len = ym_wr_index;
                    ym_wr_index = 0;
                    ym_sta = YM_STA_RX_DONE;
                }else{
                    ymodem_putchar(YM_NAK);
                    ym_len = 0;
                    ym_wr_index = 0;
                }
            }else if(ym_eot_flag == 2){
                ym_len = ym_wr_index;
                ym_wr_index = 0;
                ym_sta = YM_STA_FINISH;
             }else{
                ym_tx = YM_RET_NAK;
                ym_retry();
                ym_retry_cnt++;
                if(ym_retry_cnt>30){
                    ym_retry_cnt = 0;
                    ymodem_putstr("\r\nWait timeout, upgarde failed\r\n");
                    ymodem_putstr("\r\nFirmware erased, please try again\r\n");
                    ym_restart();
                }
            }
        }
        break;
    case YM_STA_FINISH:
        ym_wait(50);
        ymodem_putstr("\r\nFirmware upgrade successfully\r\n");
        ym_restart();
        break;
    case YM_STA_FATAL:
        ymodem_putstr("\r\nFatal error\r\n");
        ym_restart();
        break;
    }
}