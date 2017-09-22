/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include <string.h>
#include <math.h>
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"
#include "sht21.h"
#include "adc-board.h"
#include "app-test.h"
#include "checkbl.h"
#include "gollum.h"
#include "cfg.h"
#include "led.h"
#include "app.h"
#include "SHT2x.h"

#include "hal-pwm.h"

typedef enum{
    APP_STA_ABP,
    APP_STA_OTAA,
    APP_STA_CFG,
}app_sta_t;

app_sta_t app_sta;
lw_rx_info_t *app_lw_rx;
int loramac_lc_cnt;

static bool rx_flag = false;
static uint8_t rssi = 0xFF;        // +180, -150dBm -> 30dBm, rssi = 30~210;
static int8_t snr = 0xFF;
static uint16_t temp;
static uint16_t rh;
static uint16_t vdd;

/**  */
typedef enum{
    SYS_STA_JOIN,
    SYS_STA_TX,
}sys_sta_t;

static TimerEvent_t ReportTimer;
static volatile bool ReportTimerEvent = false;
static sys_sta_t sys_sta;

static uint32_t ReportDutyCycleTime;
static uint16_t period;
static uint32_t dutycycle;
static uint32_t dutycycle_oft;
static uint8_t AppData[32];
Gpio_t usb_on;

/* STE protocol */
#define STE_BUF_SIZE                    (72)
static bool ste_conf_report_flag;
static bool ste_tx_buf_flag;
static uint8_t ste_tx_buf_cnt;
static uint16_t ste_interval;
static uint16_t ste_num;
static uint32_t ste_dc_dft;
static uint32_t ste_long, ste_lat;
typedef struct{
    float t;
    float h;
    uint32_t ts;
}ste_dat_t;
ste_dat_t ste_buf[STE_BUF_SIZE];
ste_dat_t ste_dat;
int8_t ste_wr_index, ste_buf_cnt;      // FILO
uint16_t ste_frame_counter;
uint8_t pwm;

int app_set_period(int32_t t)
{
    if(t<5000){
        dutycycle = 8000;
        return -1;
    }
    dutycycle = t;
    period = dutycycle/1000/2;
    return 0;
}

uint32_t app_get_period(void)
{
    return dutycycle;
}

int app_set_period_oft(int32_t t)
{
    if(t<0){
        return -1;
    }else if(t > dutycycle/2){
        return -1;
    }
    dutycycle_oft = t;
    return 0;
}

uint32_t app_get_period_oft(void)
{
    return dutycycle_oft;
}

void update_rssi_snr(lw_sta_t sta)
{
    switch(sta){
    case LW_STA_RX:
    case LW_STA_ACK_RX:
    case LW_STA_ACK:
    case LW_STA_JOINED:
    case LW_STA_CMD_RECEIVED:
        rssi = (uint8_t)((int16_t)180+app_lw_rx->rssi);
        snr = app_lw_rx->snr;
        if(snr == -1){
            snr = 0;
        }
        rx_flag = !rx_flag;
        break;
    default:
        if( (app_lw_rx->size > 0) || (app_lw_rx->maccmd_size> 0) || (app_lw_rx->link_alive > 0) ){
            rssi = (uint8_t)((int16_t)180+app_lw_rx->rssi);
            snr = app_lw_rx->snr;
            if(snr == -1){
                snr = 0;
            }
            rx_flag = !rx_flag;
        }
        break;
    }
}

void OnReportTimerEvent( void )
{
    ReportTimerEvent = true;
}

void app_sta_set(app_sta_t sta)
{
    switch(sta){
    case APP_STA_ABP:
        app_lw_rx = lw_init((PhyType_t)cfg_read(CFG_BAND), LW_MODE_ABP);
        sys_sta = SYS_STA_JOIN;
        TimerStop(&ReportTimer);
        TimerInit( &ReportTimer, OnReportTimerEvent );
        ReportTimerEvent = true;
        loramac_lc_cnt = 0;
        break;
    case APP_STA_OTAA:
        app_lw_rx = lw_init((PhyType_t)cfg_read(CFG_BAND), LW_MODE_OTAA);
        sys_sta = SYS_STA_JOIN;
        TimerStop(&ReportTimer);
        TimerInit( &ReportTimer, OnReportTimerEvent );
        ReportTimerEvent = true;
        loramac_lc_cnt = 0;
        break;
    case APP_STA_CFG:

        break;
    default:
        return;
    }
    app_sta = sta;
}

void app_init(void)
{
    float tf, hf;
    sht_t sht;

    ste_conf_report_flag = true;
    ste_tx_buf_flag = false;
    ste_tx_buf_cnt = 0;
    ste_num = 0;
    ste_interval = (uint8_t)(1.0*dutycycle/60/1000 + 0.5);
    ste_long = cfg_read(CFG_GPS_LONG);
    ste_lat = cfg_read(CFG_GPS_LAT);
    app_sta_set((app_sta_t)cfg_read(CFG_MODE));
    ste_wr_index = 0, ste_buf_cnt = 0;
    ste_frame_counter = 0;

    if((sht_fmt_t)cfg_read(CFG_PLFMT) == SHT_FMT_STEE){
        DelayMs(50);
        sht = shtxx_read_th_float(&tf, &hf);
        if(sht != SHTNONE){
            ste_dat.ts = TimerGetCurrentTime();
            ste_dat.t = tf;
            ste_dat.h = hf;
            ste_buf[ste_wr_index] = ste_dat;
        }
    }
}

/* v in 0.01v */
uint8_t app_get_batt(uint16_t v)
{
    uint8_t ret = 100;

    if(v > 353){
        ret = 100;
    }else if(v > 330){
        ret = (uint8_t)(320*0.01*v-1036);
    }else if(v > 200){
        ret = (uint8_t)((15*0.01*v-29.5) + 0.5);
    }else{
        ret = 0;
    }
    return ret;
}

uint16_t app_get_vdd()
{
#define READ_TIMES          10

    uint32_t tmp_sum;
    uint16_t tmp[READ_TIMES];
    uint16_t tmp_min = 0xFFFF, tmp_max = 0;
    int i;

    for(i=0, tmp_sum=0; i<READ_TIMES; i++){
        tmp[i] = hal_adc_get_vdd();
        tmp_sum += tmp[i];
        if( tmp[i] < tmp_min ){
            tmp_min = tmp[i];
        }
        if( tmp[i] > tmp_max ){
            tmp_max = tmp[i];
        }
    }
    tmp_sum = (tmp_sum - tmp_min - tmp_max)/(READ_TIMES-2);
    return tmp_sum;
}

void app_update_period()
{
    if(ste_num != 0){
        ste_num--;
        if(ste_num == 0){
            dutycycle = ste_dc_dft;
        }
    }
}

int app_set_buf(uint8_t *buf, uint8_t *len, Mcps_t *mcps)
{
    int i;
    sht_t sht;
    uint8_t hdr;
    sht_fmt_t fmt;
    uint16_t ts;
    float tf, hf;
    uint32_t *ptf = (uint32_t *)&tf, *phf = (uint32_t *)&hf;

    fmt = (sht_fmt_t)cfg_read(CFG_PLFMT);

    *mcps = MCPS_UNCONFIRMED;

    if(fmt != SHT_FMT_STEE){
        DelayMs(50);
        sht = shtxx_read_th(fmt, &temp, &rh);

        if(sht == SHTNONE){
#if USE_DEBUGGER
            uint16_t tmp;
            tmp = shtxx_error();
            buf[0] = (uint8_t)(tmp>>0);
            buf[1] = (uint8_t)(tmp>>8);
            *len = 2;
            return 0;
#else
            if( fmt == SHT_FMT_SHT21 ){
                hdr = 0x01;
            }else if(fmt == SHT_FMT_STD){
                hdr = 0x02;
            }
            buf[0] = 0x00;
            /** receive new packet */
            if(rx_flag){
                buf[0] |= 0x80;
            }else{
                buf[0] &= ~0x80;
            }
            /** payload type, max fixed length */
            buf[0] = (buf[0]&0xFC) | hdr;
            buf[1] = 0xFF;
            buf[2] = 0xFF;
            buf[3] = 0xFF;
            buf[4] = (period>>0) & 0xFF;
            buf[5] = (period>>8) & 0xFF;
            buf[6] = rssi;
            buf[7] = snr;
            buf[8] = (vdd-150) & 0xFF;
            buf[9] = pwm;
            *len = 10;
            return 0;
#endif
        }

        if( fmt == SHT_FMT_SHT21 ){
            rh = ((rh+0x7f)>>8);
            hdr = 0x01;
        }else if(fmt == SHT_FMT_STD){
            hdr = 0x02;
        }
        buf[0] = 0x00;
        /** receive new packet */
        if(rx_flag){
            buf[0] |= 0x80;
        }else{
            buf[0] &= ~0x80;
        }
        /** payload type, max fixed length */
        buf[0] = (buf[0]&0xFC) | hdr;
        buf[1] = (temp>>0) & 0xFF;
        buf[2] = (temp>>8) & 0xFF;
        buf[3] = (uint8_t)(rh&0xFF);
        buf[4] = (period>>0) & 0xFF;
        buf[5] = (period>>8) & 0xFF;
        buf[6] = rssi;
        buf[7] = snr;
        buf[8] = (vdd-150) & 0xFF;
        buf[9] = pwm;
        *len = 10;
        return 0;
    }

    i = 0;
    *mcps = MCPS_CONFIRMED;
    if( ste_conf_report_flag ){
        ste_conf_report_flag = false;

        buf[i++] = 0x8F;
        buf[i++] = 17;
        buf[i++] = 0x02;
        buf[i++] = ste_interval;
        buf[i++] = (uint8_t)((ste_long>>0)&0xFF);
        buf[i++] = (uint8_t)((ste_long>>8)&0xFF);
        buf[i++] = (uint8_t)((ste_long>>16)&0xFF);
        buf[i++] = (uint8_t)((ste_long>>24)&0xFF);
        buf[i++] = (uint8_t)((ste_lat>>0)&0xFF);
        buf[i++] = (uint8_t)((ste_lat>>8)&0xFF);
        buf[i++] = (uint8_t)((ste_lat>>16)&0xFF);
        buf[i++] = (uint8_t)((ste_lat>>24)&0xFF);
        buf[i++] = 0;
        buf[i++] = 0xFF;
        buf[i++] = 0xFF;
        buf[i++] = (VER_MINOR<<4) | (VER_PATCH);
        buf[i++] = VER_MAJOR;
    }else if( ste_tx_buf_flag && (ste_buf_cnt > 0) ){
        ste_tx_buf_flag = false;

        ste_buf_cnt--;
        ste_wr_index--;
        if(ste_wr_index < 0){
            ste_wr_index = STE_BUF_SIZE-1;
        }

        ste_frame_counter++;

        ts = (uint16_t)(TimerGetElapsedTime(ste_buf[ste_wr_index].ts)/1000);
        tf = ste_buf[ste_wr_index].t;
        hf = ste_buf[ste_wr_index].h;

        buf[i++] = 0x80;
        buf[i++] = 17;
        buf[i++] = (uint8_t)((ste_frame_counter>>0)&0xFF);
        buf[i++] = (uint8_t)((ste_frame_counter>>8)&0xFF);
        buf[i++] = app_get_batt(vdd);
        buf[i++] = (uint8_t)((ts>>0)&0xFF);
        buf[i++] = (uint8_t)((ts>>8)&0xFF);
        buf[i++] = 0x02;
        buf[i++] = (uint8_t)((*ptf>>0)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>8)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>16)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>24)&0xFF);
        buf[i++] = 0x03;
        buf[i++] = (uint8_t)((*phf>>0)&0xFF);
        buf[i++] = (uint8_t)((*phf>>8)&0xFF);
        buf[i++] = (uint8_t)((*phf>>16)&0xFF);
        buf[i++] = (uint8_t)((*phf>>24)&0xFF);
    }else{
        DelayMs(50);
        sht = shtxx_read_th_float(&tf, &hf);
        if(sht == SHTNONE){
            return -1;
        }

        ste_frame_counter++;

        ste_dat.ts = TimerGetCurrentTime();
        ste_dat.t = tf;
        ste_dat.h = hf;

        buf[i++] = 0x80;
        buf[i++] = 17;
        buf[i++] = (uint8_t)((ste_frame_counter>>0)&0xFF);
        buf[i++] = (uint8_t)((ste_frame_counter>>8)&0xFF);
        buf[i++] = app_get_batt(vdd);
        buf[i++] = 0;
        buf[i++] = 0;
        buf[i++] = 0x02;
        buf[i++] = (uint8_t)((*ptf>>0)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>8)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>16)&0xFF);
        buf[i++] = (uint8_t)((*ptf>>24)&0xFF);
        buf[i++] = 0x03;
        buf[i++] = (uint8_t)((*phf>>0)&0xFF);
        buf[i++] = (uint8_t)((*phf>>8)&0xFF);
        buf[i++] = (uint8_t)((*phf>>16)&0xFF);
        buf[i++] = (uint8_t)((*phf>>24)&0xFF);
    }

    *len = i;
    return 0;
}

#define UPLINK_CNT              1
static int cnt = UPLINK_CNT;
uint32_t app_dl(lw_sta_t sta)
{
    uint32_t rtime;
    double dc;


    rtime = dutycycle + randr( -dutycycle_oft, dutycycle_oft );

#if 1
    if(app_lw_rx->size == 2){
        period = app_lw_rx->buf[1];
        period <<= 8;
        period += app_lw_rx->buf[0];
        dutycycle = (uint32_t)period*2*1000;
    }else if(app_lw_rx->size == 1){
        if(app_lw_rx->buf[0] == 0xFF){

        }else{
            pwm = app_lw_rx->buf[0];
            dc = 1.0 * app_lw_rx->buf[0] / 0xFE;
            hal_pwm_set_dc(dc);
        }
    }
    if( app_lw_rx->size != 0 ){
        cnt = UPLINK_CNT;
    }

    if( cnt != 0 ){
        cnt--;
        rtime = 100;
    }

    return rtime;
#else
    if((sht_fmt_t)cfg_read(CFG_PLFMT) != SHT_FMT_STEE){
        if(app_lw_rx->size == 2){
            period = app_lw_rx->buf[1];
            period <<= 8;
            period += app_lw_rx->buf[0];
            dutycycle = (uint32_t)period*2*1000;
        }else if(app_lw_rx->size == 1){
            if(app_lw_rx->buf[0] == 0xFF){

            }else{
                pwm = app_lw_rx->buf[0];
                dc = 1.0 * app_lw_rx->buf[0] / 0xFE;
                hal_pwm_set_dc(dc);
            }
        }
        return rtime;
    }else{
        if( (sta == LW_STA_ACK) || (sta == LW_STA_ACK_RX) ){
            /* Ack received, check if there is more data need to be sent */
            if( ste_tx_buf_cnt<6 ){
                if(ste_buf_cnt > 0){
                    ste_tx_buf_flag = true;
                    ste_tx_buf_cnt++;
                    rtime = 8*1000 + randr( -3, 3 );
                }
            }else{
                ste_tx_buf_cnt = 0;
            }
        }else{
            ste_tx_buf_cnt = 0;
            ste_buf[ste_wr_index] = ste_dat;
            /* Ack not received, set data to buffer, and wait next round */
            ste_wr_index++;
            if(ste_wr_index >= STE_BUF_SIZE){
                ste_wr_index = 0;
            }
            ste_buf_cnt++;
            if(ste_buf_cnt > STE_BUF_SIZE){
                /* Buffer overflow, data lost, keep only last STE_BUF_SIZE data */
                ste_buf_cnt = STE_BUF_SIZE;
            }
        }

        if(app_lw_rx->size > 0){
            switch(app_lw_rx->buf[0]){
            case 0x22:  // Change Alert Command

                break;
            case 0x21:  // Configuration request
                if( (app_lw_rx->size == 2) && (app_lw_rx->buf[1] == 2) ){
                    ste_conf_report_flag = true;
                }
                break;
            case 0x20:  // Change Report interval Command
                if( (app_lw_rx->size == 7) && (app_lw_rx->buf[1] == 7) && (app_lw_rx->buf[2] == 0) ){
                    ste_interval = (app_lw_rx->buf[3]<<0) | (app_lw_rx->buf[4]<<8);
                    ste_num = (app_lw_rx->buf[5]<<0) | (app_lw_rx->buf[6]<<8);
                    if(ste_num != 0){
                        ste_dc_dft = dutycycle;
                    }
                    dutycycle = ste_interval * 60 * 1000;
                }
                break;
            }

        }
    }
    return rtime;
#endif
}

void app_abp(void)
{
    int ret;
    lw_sta_t sta;
    uint8_t len;
    Mcps_t mcps;
    MlmeReq_t mlmeReq;
    MibRequestConfirm_t mibReq;

    switch(sys_sta){
    case SYS_STA_JOIN:
        mibReq.Type = MIB_NETWORK_JOINED;
        LoRaMacMibGetRequestConfirm( &mibReq );
        if(mibReq.Param.IsNetworkJoined){
            sys_sta = SYS_STA_TX;
            mibReq.Type = MIB_DEVICE_CLASS;
            mibReq.Param.Class = (DeviceClass_t)cfg_read(CFG_CLASS);
            LoRaMacMibSetRequestConfirm( &mibReq );
            break;
        }
        if(ReportTimerEvent != true){
            break;
        }
        ReportTimerEvent = false;
        ret = lw_join(LW_JOIN_NORMAL);
ABP_CLAA_CHECK_RET:
        if(ret == 0){
            TimerSetValue( &ReportTimer, APP_SEND_SUCCESS_TIMEOUT );
        }else{
            if(ret == LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR){
                ret = lw_send(mcps, (uint8_t)cfg_read(CFG_PORT), NULL, 0, cfg_read(CFG_RETRY), -1);
                goto ABP_CLAA_CHECK_RET;
            }
            TimerSetValue( &ReportTimer, APP_SEND_FAIL_TIMEOUT );
        }
        TimerStart( &ReportTimer );
        break;
    case SYS_STA_TX:
        if(loramac_lc_cnt>=APP_LINK_CHECK_LIMIT_MAX){
            if(app_sta == APP_STA_OTAA){
                sys_sta = SYS_STA_JOIN;
                app_sta_set((app_sta_t)cfg_read(CFG_MODE));
                break;
            }else{
                MibRequestConfirm_t mibReq;
                mibReq.Type = MIB_UPLINK_COUNTER;
                LoRaMacMibGetRequestConfirm( &mibReq );
                if( mibReq.Param.UpLinkCounter > 65535 ){
                    app_sta_set((app_sta_t)cfg_read(CFG_MODE));
                }
            }
        }
        if(ReportTimerEvent != true){
            break;
        }
        ReportTimerEvent = false;
        TimerSetValue( &ReportTimer, APP_SEND_FAIL_TIMEOUT );
        if( app_set_buf(AppData, &len, &mcps) == 0 ){
            if( (loramac_lc_cnt > 0 ) && (loramac_lc_cnt%APP_LINK_CHECK_LIMIT == 0) ){
                mlmeReq.Type = MLME_LINK_CHECK;
                LoRaMacMlmeRequest( &mlmeReq );
            }else if( (loramac_lc_cnt>APP_LINK_CHECK_LIMIT) && (loramac_lc_cnt<APP_LINK_CHECK_LIMIT_MAX) ){
                if((loramac_lc_cnt-APP_LINK_CHECK_LIMIT)%APP_LINK_CHECK_LIMIT_FAST == 0){
                    mlmeReq.Type = MLME_LINK_CHECK;
                    LoRaMacMlmeRequest( &mlmeReq );
                }
            }
            ret = lw_send(mcps, (uint8_t)cfg_read(CFG_PORT), AppData, len, cfg_read(CFG_RETRY), -1);
ABP_CHECK_RET:
            if( ret == 0 ){
                app_update_period();
                vdd = app_get_vdd();
                loramac_lc_cnt++;

                TimerSetValue( &ReportTimer, APP_SEND_SUCCESS_TIMEOUT );
            }else{
                if(ret == LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR){
                    ret = lw_send(mcps, (uint8_t)cfg_read(CFG_PORT), NULL, 0, cfg_read(CFG_RETRY), -1);
                    goto ABP_CHECK_RET;
                }
            }
        }
        TimerStart( &ReportTimer );
        break;
    }

    if(lw_get_evt(&sta) == true){
        __disable_irq();
        update_rssi_snr(sta);
        ReportDutyCycleTime = app_dl(sta);

        if(app_lw_rx->link_alive){
            loramac_lc_cnt = 0;
        }
        __enable_irq();

        ReportTimerEvent = false;
        TimerStop( &ReportTimer );
        TimerSetValue( &ReportTimer, ReportDutyCycleTime );
        TimerStart( &ReportTimer );
    }
}

void app_otaa(void)
{
    app_abp();
}

void usb_check_init(void)
{
    GpioInit( &usb_on, USB_ON, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    // Call back no need, interrupt is just used to wakeup MCU
    GpioSetInterrupt( &usb_on, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, NULL );
}

void usb_check_deinit(void)
{
    __disable_irq();
    GpioSetInterrupt( &usb_on, NO_IRQ, IRQ_HIGH_PRIORITY, NULL );
    __enable_irq();
}

void app_check_mode(void)
{
    switch(app_sta){
    case APP_STA_ABP:
    case APP_STA_OTAA:
        if(GpioRead(&usb_on) == 1){
            DelayMs(10);
            if(GpioRead(&usb_on) == 1){
                usb_check_deinit();
                gm_init();
                lw_init((PhyType_t)cfg_read(CFG_BAND), LW_MODE_ABP);
                app_sta_set(APP_STA_CFG);
            }
        }
        break;
    case APP_STA_CFG:
        if(GpioRead(&usb_on) != 1){
            DelayMs(10);
            if(GpioRead(&usb_on) != 1){
                usb_check_init();
                gm_deinit();
//                hal_i2c_deinit(&i2c1, I2C_SCL, I2C_SDA);
//                hal_i2c_init(&i2c1, I2C_SCL, I2C_SDA);
//                hal_i2c_frequency(&i2c1, 400000);
                I2c_Deinit();
                I2c_Init();
                app_init();
            }
        }
        break;
    }
}

void app_evt(void)
{
    switch(app_sta){
    case APP_STA_ABP:
        app_abp();
        //TimerLowPowerHandler( );
        break;
    case APP_STA_OTAA:
        app_otaa();
        //TimerLowPowerHandler( );
        break;
    case APP_STA_CFG:
        gm_sta_evt();
        break;
    }
}

int main( void )
{
    app_test_rf_config_t rfcfg;
    uint8_t test;

    clock_init(0);
    BoardInitMcu( );
    BoardInitPeriph( );

#ifdef USE_BOOTLOADER
    if(cbl()<0){
        while(1){
            /** LED blink */
            TimerLowPowerHandler( );
        }
    }
#endif

    led_init();

    led_on(LED1);
    cfg_init();
    led_off(LED1);

    test = cfg_read(CFG_TEST);
    if(test != 0){
        /** Test mode */
        usb_check_deinit();
        gm_init();
        app_test_init();
        cfg_get_rfcfg(&rfcfg);
        app_test_set_rf_config(&rfcfg);
        switch(test){
        case 1:
            app_test_set_sta(APP_TEST_STA_TXCW, NULL);
            break;
        case 2:
            app_test_set_sta(APP_TEST_STA_TXCLORA, NULL);
            break;
        case 3:
            app_test_set_sta(APP_TEST_STA_RXLRPKT, NULL);
            break;
        }
        while(1){
            gm_sta_evt();
            app_test_evt();
        }
    }

    app_init();
    usb_check_init();

    vdd = 0;
    vdd = app_get_vdd();

    hal_pwm_init();

    while(1){
        app_check_mode();
        app_evt();
    }
}
