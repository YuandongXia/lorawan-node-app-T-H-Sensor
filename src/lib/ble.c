/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "ble.h"

#define BLE_CMD_BUF_LEN         32
#define BLE_PKT_MAX_NUM         8

Gpio_t ble_int;

typedef enum{
    BLE_RX_STA_RECIEVE_HEAD,
    BLE_RX_STA_RECIEVING,
    BLE_RX_STA_COMPLETE,
    BLE_RX_STA_ERROR,
    BLE_RX_STA_CHAR_INVALID,
    BLE_RX_STA_BUF_OVF,
    BLE_RX_STA_TIMEOUT
}ble_rx_sta_type;

static volatile uint8_t ble_rx_buf[BLE_CMD_BUF_LEN]= {0};
static TimerEvent_t ble_rx_timer;
static volatile uint8_t ble_rx_wr_index, ble_rx_cnt;

ble_rx_sta_type ble_rx_sta;

int8_t ble_rssi;
uint8_t ble_uuid[6];
uint8_t ble_sta;

ble_pkt_t ble_pkt[BLE_PKT_MAX_NUM] = {0};
uint8_t ble_pkt_cnt;
uint8_t ble_pkt_rd_index;
uint8_t ble_pkt_wr_index;

static TimerEvent_t ble_scan_tof_timer;
uint8_t ble_scan_tof;

void ble_int_irq(void)
{
    /* Use to activate MCU */

    ble_sta = 1;
}

void ble_rx_timeout(void)
{
    /** TIMEOUT, switch to timeout status */
    ble_rx_sta=BLE_RX_STA_TIMEOUT;
    ble_rx_wr_index = 0;
}

void ble_scan_tof_evt(void)
{
    ble_scan_tof = 1;
}


/* AA 07 14 01 80 FA 3B A4 BA D9 FF */
void ble_receive_byte(uint8_t data)
{
    uint8_t tmp;
    switch(ble_rx_sta) {
    case BLE_RX_STA_RECIEVE_HEAD:
        ble_rx_buf[ble_rx_wr_index] = data;
        ble_rx_wr_index++;
        if( data == 0xAA ){
            ble_rx_sta = BLE_RX_STA_RECIEVING;
            /** start counting timeout */
            TimerStart( &ble_rx_timer );
        }else{
            ble_rx_wr_index = 0;
        }
        break;
    case BLE_RX_STA_RECIEVING:
        /** recieve fresh data, restart timer counter*/
        TimerStop( &ble_rx_timer );
        TimerStart( &ble_rx_timer );

        ble_rx_buf[ble_rx_wr_index] = data;
        ble_rx_wr_index++;
        tmp = ble_rx_wr_index ;
        if( tmp == (ble_rx_buf[1] + 4) ){
            ble_rx_sta = BLE_RX_STA_COMPLETE;
            TimerStop( &ble_rx_timer );
        }

        /** Buffer overflow */
        if(ble_rx_wr_index >= BLE_CMD_BUF_LEN){
            /** atmunication buffer overflow */
            ble_rx_sta = BLE_RX_STA_BUF_OVF;
            /** recieve timeout close */
            TimerStop( &ble_rx_timer );
            /** reset recieve pointer */
            ble_rx_wr_index = 0;
        }
        break;
    default:

        break;
    }
}

int ble_frame_analyse(uint8_t *buf, int len)
{
    int i;
    uint8_t sum;

    if(buf[0] != 0xAA || buf[len-1] != 0xFF){
        return -1;
    }

    for(i=0, sum=0; i<len-2; i++){
        sum+=buf[i];
    }

    if(buf[len-2] != sum ){
        return -2;
    }

    ble_rssi = buf[8];

    for(i=0; i<6; i++){
        ble_uuid[i] =  buf[7-i];
    }

    ble_pkt[ble_pkt_wr_index].rssi = ble_rssi;
    memcpy(ble_pkt[ble_pkt_wr_index].uuid, ble_uuid, 6);
    ble_pkt_wr_index++;
    if(ble_pkt_wr_index >= BLE_PKT_MAX_NUM){
        ble_pkt_wr_index = 0;
    }

    ble_pkt_cnt++;
    if(ble_pkt_cnt > BLE_PKT_MAX_NUM){
        /* one ble packet will lose */
        ble_pkt_cnt = BLE_PKT_MAX_NUM;
    }

    return 0;
}

int ble_sta_handle(void)
{
    int ret = 0;

    switch(ble_rx_sta) {
    case BLE_RX_STA_COMPLETE:
        if(0==ble_frame_analyse((uint8_t *)ble_rx_buf, ble_rx_wr_index)){
            ret = 1;
        }
        /** wait new packet */
        ble_rx_sta=BLE_RX_STA_RECIEVE_HEAD;
        ble_rx_wr_index = 0;
        ble_sta = 0;

        ble_scan_tof = 0;
        TimerStop( &ble_scan_tof_timer );
        TimerStart( &ble_scan_tof_timer );
        break;

    case BLE_RX_STA_RECIEVE_HEAD:
        /** do nothing, handle in interrupt, maybe can do some check */
        break;
    case BLE_RX_STA_RECIEVING:
        /** use time to do timeout check */
        break;
    //case AT_RX_STA_PACKET_OVF:
    case BLE_RX_STA_BUF_OVF:
    case BLE_RX_STA_CHAR_INVALID:
    case BLE_RX_STA_ERROR:
    case BLE_RX_STA_TIMEOUT:
        ble_rx_sta=BLE_RX_STA_RECIEVE_HEAD;
        ble_rx_wr_index = 0;
        ble_sta = 0;
        break;
    }

    return ret;
}

void ble_init(void)
{
    uart_config_t uart_config;

    GpioInit( &ble_int, BT_INT, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
    GpioSetInterrupt( &ble_int, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, ble_int_irq );

    uart_config.baud = 9600;
    uart_config.word_length = 8;
    uart_config.parity = NONE;
    uart_config.stop_bits = 1;
    uart_init(&uart_config, ble_receive_byte);

    TimerInit( &ble_rx_timer, ble_rx_timeout );
    TimerSetValue( &ble_rx_timer, 50000);

    TimerInit( &ble_scan_tof_timer, ble_scan_tof_evt );
    TimerSetValue( &ble_scan_tof_timer, 35000000);
    TimerStart( &ble_scan_tof_timer );
    ble_scan_tof = 0;

    ble_rx_sta=BLE_RX_STA_RECIEVE_HEAD;
    ble_sta = 0;

    ble_pkt_cnt = 0;
    ble_pkt_rd_index = 0;
    ble_pkt_wr_index = 0;
}

int8_t ble_get_rssi(void)
{
    return ble_rssi;
}

int8_t ble_new(void)
{
    return (ble_scan_tof == 0);
}

void ble_get_uuid(uint8_t *buf, int len)
{
    memcpy(buf, ble_uuid, len);
}

int ble_readable(void)
{
    return ble_pkt_cnt;
}

int ble_read(uint8_t *uuid, int8_t *rssi)
{
    if(ble_pkt_cnt<=0){
        return -1;
    }

    __disable_irq();
    *rssi = ble_pkt[ble_pkt_rd_index].rssi;
    memcpy(uuid, ble_pkt[ble_pkt_rd_index].uuid, 6);
    ble_pkt_rd_index++;
    if(ble_pkt_rd_index >= BLE_PKT_MAX_NUM){
        ble_pkt_rd_index = 0;
    }
    ble_pkt_cnt--;
    __enable_irq();

    return 0;
}

/* 1: busy, 0: idle */
int ble_busy(void)
{
    if(GpioRead(&ble_int) == 0){
        return 1;
    }

    return 0;
}

