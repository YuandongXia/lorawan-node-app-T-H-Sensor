/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "gollum.h"
#include "cmd.h"
#include "timer.h"

#ifdef USE_USB
#include "usb-cdc.h"
#endif

#define GM_BUF_LEN                      (512)
#define GM_TIMEOUT_MIN                  (300000)

//#define GM_UNCHECK_CRC

typedef enum{
    GM_STA_RX_HEAD,
    GM_STA_RX_LEN,
    GM_STA_RX,
    GM_STA_RX_DONE,
    GM_STA_RX_ERROR,
    GM_STA_RX_TIMEOUT,
}gm_sta_t;

typedef enum{
    GM_TIMER_IDLE,
    GM_TIMER_STOP,
    GM_TIMER_START,
}gm_tflag_t;

static gm_sta_t gm_sta;
static uint16_t gm_len;
static uint8_t gm_buf[GM_BUF_LEN];
//static uint8_t gm_tx_buf[GM_BUF_LEN];
static volatile uint16_t gm_index;

static TimerEvent_t gm_timer;
static uint32_t gm_tvalue;
static gm_tflag_t gm_tflag;
static bool gm_timer_restart;

static uint16_t gm_crc(uint8_t *buf, int len);
static uint16_t gm_cmd_crc(uint8_t cmd, int8_t code, uint8_t *buf, int16_t len);

void gm_timeout(void)
{
    if(gm_timer_restart == true){
        gm_timer_restart = false;
        TimerStart(&gm_timer);
        return;
    }
    /** TIMEOUT, switch to timeout status */
    gm_sta=GM_STA_RX_TIMEOUT;
}

static void gm_rx_byte(uint8_t dt)
{
    switch(gm_sta){
    case GM_STA_RX_HEAD:
        if(dt == GM_CODE_HEAD){
            gm_buf[0] = dt;
            gm_index = 1;
            gm_sta = GM_STA_RX_LEN;
            gm_tflag = GM_TIMER_START;
        }
        break;
    case GM_STA_RX_LEN:
        gm_timer_restart = true;
        gm_buf[gm_index] = dt;
        gm_index++;
        if(gm_index == GM_OFT_CMD){
            gm_len = (((uint16_t)gm_buf[GM_OFT_LEN0])<<0) | (((uint16_t)gm_buf[GM_OFT_LEN1])<<8);
            if(gm_len > (GM_BUF_LEN-5)){
                gm_sta = GM_STA_RX_ERROR;
                /** recieve timeout close */
                gm_tflag = GM_TIMER_STOP;
                /** reset recieve pointer */
                gm_index = 0;
            }else{
                gm_sta = GM_STA_RX;
            }
        }
        break;
    case GM_STA_RX:
        gm_timer_restart = true;
        gm_buf[gm_index] = dt;
        gm_index++;
        if((gm_len+5) == gm_index){
            gm_sta = GM_STA_RX_DONE;
            gm_tflag = GM_TIMER_STOP;
        }else{

        }
        if(gm_index >= GM_BUF_LEN){
            /** atmunication buffer overflow */
            gm_sta = GM_STA_RX_ERROR;
            /** recieve timeout close */
            gm_tflag = GM_TIMER_STOP;
            /** reset recieve pointer */
            gm_index = 0;
        }

        break;
    default:
        break;
    }
}

void gm_ack(uint8_t cmd, int8_t code)
{
    gm_tx(cmd, code, NULL, 0);
}

void gm_tx(uint8_t cmd, int8_t code, uint8_t *buf, int16_t len)
{
    int index = GM_OFT_HEAD;
    uint16_t crc;
    int i;
    uint8_t head[5];
    uint8_t crcbuf[5];

    head[index++] = GM_CODE_HEAD;
    head[index++] = (uint8_t)(len+2);
    head[index++] = (uint8_t)((len+2)>>8);
    head[index++] = cmd;
    head[index++] = code;
    crc = gm_cmd_crc(cmd, code, buf, len);
    crcbuf[0] = (uint8_t)(crc>>0);
    crcbuf[1] = (uint8_t)(crc>>8);

    gm_putbuf(head, 5);
    /** in case of gm_putbuf can only accept 255 bytes data */
    i=0;
    while(i<len){
        if( (len-i)>255 ){
            gm_putbuf(buf+i, 255);
            i+=255;
        }else{
            gm_putbuf(buf+i, (len-i));
            i=len;
        }
    }
    gm_putbuf(crcbuf, 2);
}

static int gm_check_crc(uint8_t *buf, int len)
{
    uint16_t crc;
    crc = gm_crc(buf, len-2) ^ ((((uint16_t)buf[len-2])<<0) | (((uint16_t)buf[len-1])<<8));

    if(crc == 0){
        return 0;
    }

#ifdef GM_UNCHECK_CRC
    return 0;
#else
    return -1;
#endif
}

void gm_sta_evt(void)
{
    if(gm_tvalue != 0){
        gm_tflag_t flag;
        __disable_irq();
        flag = gm_tflag;
        gm_tflag = GM_TIMER_IDLE;
        __enable_irq();
        switch(flag){
        case GM_TIMER_STOP:
            gm_timer_restart = false;
            TimerStop(&gm_timer);
            break;
        case GM_TIMER_START:
            TimerStart(&gm_timer);
            break;
        }
        if(gm_sta == GM_STA_RX_TIMEOUT){

        }
    }else{
        gm_tflag = GM_TIMER_IDLE;
    }

    switch(gm_sta){
    case GM_STA_RX_DONE:
#ifdef GM_DEBUG
        //gm_putbuf(gm_buf, gm_index);
#endif
        if(gm_check_crc(gm_buf, gm_index) == 0){
            if(cmd_exec(gm_buf+GM_OFT_CMD, gm_index-5)<0){
                gm_ack(GM_CODE_CMD_ERROR, ERR_CMD_UNKNOWN);
            }
        }else{
            gm_ack(GM_CODE_CMD_ERROR, ERR_CRC);
        }
        gm_sta=GM_STA_RX_HEAD;
        gm_index = 0;
        break;
    case GM_STA_RX_HEAD:
    case GM_STA_RX_LEN:
    case GM_STA_RX:
        break;
    case GM_STA_RX_ERROR:
        gm_ack(GM_CODE_CMD_ERROR, ERR_FRAME);
        gm_sta=GM_STA_RX_HEAD;
        gm_index = 0;
        break;
    case GM_STA_RX_TIMEOUT:
        gm_ack(GM_CODE_CMD_ERROR, ERR_TIMEOUT);
        gm_sta=GM_STA_RX_HEAD;
        gm_index = 0;
        break;
    default:
        break;
    }
}

void gm_init(void)
{
#ifdef USE_USB
    UsbMcuInit();
    usb_cdc_init(gm_rx_byte);
#else
    uartd_config_t uart_config;
    uart_config.baud = 115200;
    uart_config.word_length = 8;
    uart_config.parity = NONE;
    uart_config.stop_bits = 1;
    uartd_init(&uart_config, gm_rx_byte);
#endif

    gm_sta = GM_STA_RX_HEAD;

    gm_tvalue = GM_TIMEOUT_MIN;
    TimerInit( &gm_timer, gm_timeout );
    TimerSetValue( &gm_timer, gm_tvalue);
    gm_tflag = GM_TIMER_IDLE;
}

void gm_deinit(void)
{
#ifdef USE_USB
    UsbMcuDeInit();
#else
    uartd_deinit(0);
#endif
}

static uint16_t gm_crc(uint8_t *buf, int len)
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

static uint16_t gm_cmd_crc(uint8_t cmd, int8_t code, uint8_t *buf, int16_t len)
{
    uint16_t crc = 0;
    uint16_t i = 0, j=0;
    uint8_t head[5];
    int index = 0;

    head[index++] = GM_CODE_HEAD;
    head[index++] = (uint8_t)(len+2);
    head[index++] = (uint8_t)((len+2)>>8);
    head[index++] = cmd;
    head[index++] = code;

    for(j=0; j<5; j++){
        crc = crc^(int)(head[j]) << 8;
        for (i=8; i!=0; i--){
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
    }

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