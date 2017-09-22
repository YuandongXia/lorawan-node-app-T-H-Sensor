/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "cfg.h"
#include "cmd.h"
#include "board.h"
#include "str2hex.h"
#include "checkbl.h"
#include "timer.h"
#include "LoRaMac.h"
#include "app-test.h"
#include "app.h"

#ifndef DISABLE_CMD_GPIO
#include "ugpio.h"
#endif

#define CMD_MAX_NUM                     (sizeof(cmd_list)/sizeof(cmd_list_t))

#define CMD_TEST_MAX_NUM                12
#define CMD_BUF_SIZE                    512

#define CMD_FDEFAULT_PASSWD             "RisingHF"

int cmd_reset(uint8_t *cmd_argv, int len);
int cmd_ver(uint8_t *cmd_argv, int len);
int cmd_id(uint8_t *cmd_argv, int len);
int cmd_adr(uint8_t *cmd_argv, int len);
int cmd_dr(uint8_t *cmd_argv, int len);
int cmd_rept(uint8_t *cmd_argv, int len);
int cmd_power(uint8_t *cmd_argv, int len);
int cmd_ch(uint8_t *cmd_argv, int len);
int cmd_rxwin1(uint8_t *cmd_argv, int len);
int cmd_rxwin2(uint8_t *cmd_argv, int len);
int cmd_key(uint8_t *cmd_argv, int len);
int cmd_dfu(uint8_t *cmd_argv, int len);
int cmd_mode(uint8_t *cmd_argv, int len);
int cmd_class(uint8_t *cmd_argv, int len);
int cmd_port(uint8_t *cmd_argv, int len);
int cmd_fdefault(uint8_t *cmd_argv, int len);
int cmd_period(uint8_t *cmd_argv, int len);
int cmd_test(uint8_t *cmd_argv, int len);
int cmd_gpio(uint8_t *cmd_argv, int len);
int cmd_sleep(uint8_t *cmd_argv, int len);
int cmd_threshold(uint8_t *cmd_argv, int len);
int cmd_ulfmt(uint8_t *cmd_argv, int len);
int cmd_gps(uint8_t *cmd_argv, int len);
int cmd_mc(uint8_t *cmd_argv, int len);

const cmd_list_t cmd_list[] = {
    {CMD_VER,           cmd_ver},
    {CMD_ID,            cmd_id},
    {CMD_ADR,           cmd_adr},
    {CMD_DR,            cmd_dr},
    {CMD_REPT,          cmd_rept},
    {CMD_POWER,         cmd_power},
    {CMD_CH,            cmd_ch},
    {CMD_RXWIN1,        cmd_rxwin1},
    {CMD_RXWIN2,        cmd_rxwin2},
    {CMD_KEY,           cmd_key},
    {CMD_MODE,          cmd_mode},
    {CMD_CLASS,         cmd_class},
    {CMD_PORT,          cmd_port},
    {CMD_FDEFAULT,      cmd_fdefault},
    {CMD_PERIOD,        cmd_period},
#ifndef RHF1S003
    {CMD_TEST,          cmd_test},
#endif
    {CMD_THRESHOLD,     cmd_threshold},
#ifndef RHF1S003
    {CMD_ULFMT,         cmd_ulfmt},
#endif
    {CMD_GPS,           cmd_gps},
    {CMD_MC,            cmd_mc},

//    {CMD_GPIO,          cmd_gpio},
//    {CMD_SLEEP,         cmd_sleep},
};

uint8_t cmd_buf[CMD_BUF_SIZE];

static MibRequestConfirm_t mibReq;
MulticastParams_t cmd_mc_list;

void cmd_cpy(uint8_t *distbuf, uint8_t *origbuf, int len)
{
    int i;
    for(i=0; i<len; i++){
        distbuf[i] = origbuf[len-i-1];
    }
}

int cmd_id(uint8_t *cmd_argv, int len)
{
    uint8_t buf[16];
    int i;

    if(len==1){
        goto RETID;
    }

    if(len>=CMD_MIN_LEN){
        /** Check if parameter is valid, start from payload */
        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_ID_SUB_SET_DEVADDR:
                if((len-i) < CMD_ID_LEN_SET_DEVADDR){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_ID_LEN_SET_DEVADDR;
                break;
            case CMD_ID_SUB_SET_DEVEUI:
                if((len-i) < CMD_ID_LEN_SET_DEVEUI){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_ID_LEN_SET_DEVEUI;
                break;
            case CMD_ID_SUB_SET_APPEUI:
                if((len-i) < CMD_ID_LEN_SET_APPEUI){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_ID_LEN_SET_APPEUI;
                break;
            case CMD_ID_SUB_CHK:
                if(len != CMD_MIN_LEN){
                    return ERR_PARA_INVALID;
                }
RETID:
                i=0;
                mibReq.Type = MIB_DEV_ADDR;
                LoRaMacMibGetRequestConfirm( &mibReq );
                cmd_buf[i++] = (uint8_t)(mibReq.Param.DevAddr>>0);
                cmd_buf[i++] = (uint8_t)(mibReq.Param.DevAddr>>8);
                cmd_buf[i++] = (uint8_t)(mibReq.Param.DevAddr>>16);
                cmd_buf[i++] = (uint8_t)(mibReq.Param.DevAddr>>24);

                mibReq.Type = MIB_DEV_EUI;
                LoRaMacMibGetRequestConfirm( &mibReq );
                cmd_cpy(cmd_buf+i, (uint8_t *)mibReq.Param.DevEui, CMD_ID_LEN_SET_DEVEUI);
                i+=CMD_ID_LEN_SET_DEVEUI;

                mibReq.Type = MIB_APP_EUI;
                LoRaMacMibGetRequestConfirm( &mibReq );
                cmd_cpy(cmd_buf+i, (uint8_t *)mibReq.Param.AppEui, CMD_ID_LEN_SET_APPEUI);
                i+=CMD_ID_LEN_SET_APPEUI;

                gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, i);
                return CMD_OK;
            default:
                return ERR_PARA_INVALID;
            }
        }

        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_ID_SUB_SET_DEVADDR:
                cmd_cpy(buf, cmd_argv+i, CMD_ID_LEN_SET_DEVADDR);
                mibReq.Type = MIB_DEV_ADDR;
                mibReq.Param.DevAddr = (((uint32_t)buf[0])<<24) | (((uint32_t)buf[1])<<16) | \
                            (((uint32_t)buf[2])<<8) | (((uint32_t)buf[3])>>0);
                LoRaMacMibSetRequestConfirm( &mibReq );
                if( cfg_write_buf(CFG_DEVADDR, buf, CMD_ID_LEN_SET_DEVADDR) < 0 ){
                    return ERR_EEPROM;
                }
                i+=CMD_ID_LEN_SET_DEVADDR;
                break;
            case CMD_ID_SUB_SET_DEVEUI:
                cmd_cpy(buf, cmd_argv+i, CMD_ID_LEN_SET_DEVEUI);
                mibReq.Type = MIB_DEV_EUI;
                mibReq.Param.DevEui = buf;
                LoRaMacMibSetRequestConfirm( &mibReq );
                if( cfg_write_buf(CFG_DEVEUI, buf, CMD_ID_LEN_SET_DEVEUI) < 0 ){
                    return ERR_EEPROM;
                }
                i+=CMD_ID_LEN_SET_DEVEUI;
                break;
            case CMD_ID_SUB_SET_APPEUI:
                cmd_cpy(buf, cmd_argv+i, CMD_ID_LEN_SET_APPEUI);
                mibReq.Type = MIB_APP_EUI;
                mibReq.Param.AppEui = buf;
                LoRaMacMibSetRequestConfirm( &mibReq );
                if( cfg_write_buf(CFG_APPEUI, buf, CMD_ID_LEN_SET_DEVEUI) < 0 ){
                    return ERR_EEPROM;
                }
                i+=CMD_ID_LEN_SET_APPEUI;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_period(uint8_t *cmd_argv, int len)
{
    int i;
    uint32_t tmp;
    if(len==1){
        i=0;
        tmp = app_get_period()*1000;
        cmd_buf[i++] = (uint8_t)(tmp>>0);
        cmd_buf[i++] = (uint8_t)(tmp>>8);
        cmd_buf[i++] = (uint8_t)(tmp>>16);
        cmd_buf[i++] = (uint8_t)(tmp>>24);
        tmp = app_get_period_oft()*1000;
        cmd_buf[i++] = (uint8_t)(tmp>>0);
        cmd_buf[i++] = (uint8_t)(tmp>>8);
        cmd_buf[i++] = (uint8_t)(tmp>>16);
        cmd_buf[i++] = (uint8_t)(tmp>>24);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, i);
        return CMD_OK;
    }

    if(len>=CMD_MIN_LEN){
        /** Check if parameter is valid, start from payload */
        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_PERIOD_SUB_PER:
                if((len-i) < CMD_PERIOD_LEN_PER){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_PERIOD_LEN_PER;
                break;
            case CMD_PERIOD_SUB_OFT:
                if((len-i) < CMD_PERIOD_LEN_OFT){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_PERIOD_LEN_OFT;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }

        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_PERIOD_SUB_PER:
                tmp = (((uint32_t)cmd_argv[i+3])<<24) | (((uint32_t)cmd_argv[i+2])<<16) | \
            (((uint32_t)cmd_argv[i+1])<<8) | (((uint32_t)cmd_argv[i])<<0);
                tmp /= 1000;
                if(app_set_period((int32_t)tmp) < 0){
                    return ERR_API;
                }
                if( cfg_write(CFG_PERIOD, tmp) < 0 ){
                    return ERR_EEPROM;
                }

                i+=CMD_PERIOD_LEN_PER;
                break;
            case CMD_PERIOD_SUB_OFT:
                tmp = (((uint32_t)cmd_argv[i+3])<<24) | (((uint32_t)cmd_argv[i+2])<<16) | \
            (((uint32_t)cmd_argv[i+1])<<8) | (((uint32_t)cmd_argv[i])<<0);
                tmp /= 1000;
                if(app_set_period_oft((int32_t)tmp) < 0){
                    return ERR_API;
                }
                if( cfg_write(CFG_PEROFT, tmp) < 0 ){
                    return ERR_EEPROM;
                }
                i+=CMD_PERIOD_LEN_OFT;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_ver(uint8_t *cmd_argv, int len)
{
    if(len == 1){
        cmd_buf[0] = VER_MAJOR;
        cmd_buf[1] = VER_MINOR;
        cmd_buf[2] = VER_PATCH;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 3);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_fdefault(uint8_t *cmd_argv, int len)
{
    if(len == CMD_FDEFAULT_LEN){
        if( 0 != memcmp(cmd_argv+1, CMD_FDEFAULT_PASSWD ,8) ){
            return ERR_PARA_INVALID;
        }

        /* Erase CFG_FLAG */
        if( cfg_write(CFG_FLAG, 0) < 0 ){
            return ERR_EEPROM;
        }

        /* Reset configuration */
        cfg_init();
        lw_init((PhyType_t)cfg_read(CFG_BAND), (lw_mode_t)cfg_read(CFG_MODE));

        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_adr(uint8_t *cmd_argv, int len)
{
    bool sta;

    if(len == 1){
        mibReq.Type = MIB_ADR;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[0] = mibReq.Param.AdrEnable;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        sta = cmd_argv[1];
        if( cfg_write(CFG_ADR, sta) < 0 ){
            return ERR_EEPROM;
        }
        mibReq.Type = MIB_ADR;
        mibReq.Param.AdrEnable = (bool)sta;
        LoRaMacMibSetRequestConfirm( &mibReq );
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

#ifndef RHF1S003
int cmd_ulfmt(uint8_t *cmd_argv, int len)
{
    if(len == 1){
        cmd_buf[0] = cfg_read(CFG_PLFMT);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ULFMT_LEN){
        if(cmd_argv[1] > SHT_FMT_DIR){
            return ERR_PARA_INVALID;
        }
        if( cfg_write(CFG_PLFMT, cmd_argv[1]) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}
#endif

#define CMD_BAND_EU868                  0
#define CMD_BAND_EU433                  1
#define CMD_BAND_US915                  2
#define CMD_BAND_US915HYBRID            3
#define CMD_BAND_CN779                  4
#define CMD_BAND_AU915                  5
#define CMD_BAND_CUSTOM                 6
#define CMD_BAND_STE920                 7
#define CMD_BAND_AU915OLD               8
#define CMD_BAND_CN470                  9
#define CMD_BAND_AS923                  10
#define CMD_BAND_KR920                  11
#define CMD_BAND_IN865                  12
#define CMD_BAND_CN470PREQUEL           13
/* alias */
#define CMD_BAND_EU434                  14
#define CMD_BAND_AU920                  15
#define CMD_BAND_CN780                  16

const uint8_t cmd_device_to_rcfg_band_map[] = {
    CMD_BAND_EU868,
    CMD_BAND_US915,
    CMD_BAND_US915HYBRID,
    CMD_BAND_CN779,
    CMD_BAND_EU433,
    CMD_BAND_AU915,
    CMD_BAND_AU915OLD,
    CMD_BAND_CN470,
    CMD_BAND_AS923,
    CMD_BAND_KR920,
    CMD_BAND_IN865,
    CMD_BAND_CN470PREQUEL,
    CMD_BAND_STE920,
};

const PhyType_t cmd_rcfg_to_device_band_map[] = {
    EU868,
    EU433,
    US915,
    US915HYBRID,
    CN779,
    AU915,
    EU868,
    STE920,
    AU915OLD,
    CN470,
    AS923,
    KR920,
    IN865,
    CN470PREQUEL,

    EU433,
    AU915,
    CN779,
};

int cmd_dr(uint8_t *cmd_argv, int len)
{
    int dr, band, i;
    int8_t warn = 0;

    if(len == 1){
RETDR:
        mibReq.Type = MIB_PHY_TYPE;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[0] = cmd_device_to_rcfg_band_map[mibReq.Param.PhyType];

        mibReq.Type = MIB_CHANNELS_DEFAULT_DATARATE;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[1] = mibReq.Param.ChannelsDefaultDatarate;

        mibReq.Type = MIB_CHANNELS_DATARATE;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[2] = mibReq.Param.ChannelsDatarate;

        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 3);
        return CMD_OK;
    }

    if(len > 1){
        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_DR_SUB_CHECK:
                if(len != i){
                    return ERR_PARA_INVALID;
                }
                goto RETDR;
            case CMD_DR_SUB_DR:
                if((len-i) < CMD_DR_LEN_DR){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_DR_LEN_DR;
                break;
            case CMD_DR_SUB_BAND:
                if((len-i) < CMD_DR_LEN_BAND){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_DR_LEN_BAND;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }

        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_DR_SUB_DR:
                dr = cmd_argv[i];
                mibReq.Type = MIB_CHANNELS_DEFAULT_DATARATE;
                mibReq.Param.ChannelsDefaultDatarate = dr;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK ){
                    return ERR_API;
                }

                mibReq.Type = MIB_CHANNELS_DATARATE;
                mibReq.Param.ChannelsDatarate = dr;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) != LORAMAC_STATUS_OK ){
                    return ERR_API;
                }
                if( cfg_write(CFG_DR, dr) < 0 ){
                    return ERR_API;
                }
                i+=CMD_DR_LEN_DR;
                break;
            case CMD_DR_SUB_BAND:
                band = cmd_argv[i];
                if( band < (sizeof(cmd_rcfg_to_device_band_map)/sizeof(PhyType_t)) ){
                    band = cmd_rcfg_to_device_band_map[band];
                    /** Set BAND */
                    mibReq.Type = MIB_PHY_TYPE;
                    mibReq.Param.PhyType = (PhyType_t)band;
                    if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
                        return ERR_API;
                    }
                    cfg_update_band((PhyType_t)band);
                    lw_init((PhyType_t)cfg_read(CFG_BAND), (lw_mode_t)cfg_read(CFG_MODE));
                }else{
                    return ERR_PARA_INVALID;
                }
                i+=CMD_DR_LEN_BAND;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }

        gm_ack(cmd_argv[CMD_OFT_CMD], warn);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_rept(uint8_t *cmd_argv, int len)
{
    uint8_t dt;

    if(len == 1){
        mibReq.Type = MIB_CHANNELS_NB_REP;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[0] = mibReq.Param.ChannelNbRep;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        dt = cmd_argv[1];
        mibReq.Type = MIB_CHANNELS_NB_REP;
        mibReq.Param.ChannelNbRep = dt;
        if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
            return ERR_API;
        }
        if( cfg_write(CFG_REPT, dt) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_port(uint8_t *cmd_argv, int len)
{
    uint8_t dt;

    if(len == 1){
        cmd_buf[0] = (uint8_t)cfg_read(CFG_PORT);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        dt = cmd_argv[1];
        cfg_write(CFG_PORT, dt);
        if( cfg_write(CFG_PORT, dt) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_rxwin2(uint8_t *cmd_argv, int len)
{
    uint32_t freq;
    uint8_t dr, sf;
    int bw;

    if(len == 1){
        mibReq.Type = MIB_RX2_CHANNEL;
        LoRaMacMibGetRequestConfirm( &mibReq );
        freq = mibReq.Param.Rx2Channel.Frequency;
        dr = mibReq.Param.Rx2Channel.Datarate;
        sf = mibReq.Param.Rx2Channel.DrValue & 0x0F;
        bw = (mibReq.Param.Rx2Channel.DrValue >> 4) & 0x0F;

        sf = sf;
        dr = dr;
        bw = bw;

        cmd_buf[0] = (uint8_t)(freq>>0);
        cmd_buf[1] = (uint8_t)(freq>>8);
        cmd_buf[2] = (uint8_t)(freq>>16);
        cmd_buf[3] = (uint8_t)(freq>>24);
        cmd_buf[4] = dr;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 5);
        return CMD_OK;
    }

    if(len == 6){
        freq = (((uint32_t)cmd_argv[4])<<24) | (((uint32_t)cmd_argv[3])<<16) | \
            (((uint32_t)cmd_argv[2])<<8) | (((uint32_t)cmd_argv[1])<<0);
        dr = cmd_argv[5];
        mibReq.Type = MIB_RX2_CHANNEL;
        mibReq.Param.Rx2Channel.Frequency = freq;
        mibReq.Param.Rx2Channel.Datarate = dr;
        mibReq.Param.Rx2Channel.DrValue = DR_RFU;
        if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
            return ERR_API;
        }
        if( cfg_write(CFG_RXWIN2_FREQ, freq) < 0 ){
            return ERR_API;
        }
        if( cfg_write(CFG_RXWIN2_DR, dr) < 0 ){
            return ERR_API;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_rxwin1(uint8_t *cmd_argv, int len)
{
    int i, index, freq, ch;
    if(len == 1){
        for(i=0, index=0; i<LORA_MAX_NB_CHANNELS; i++){
            mibReq.Type = MIB_CUSTOM_RXWIN1;
            mibReq.Param.RxWin1.Ch = i;
            LoRaMacMibGetRequestConfirm( &mibReq );
            freq = mibReq.Param.RxWin1.Freq;
            if(freq != 0){
                cmd_buf[index++] = i;
                cmd_buf[index++] = (uint8_t)(freq>>0);
                cmd_buf[index++] = (uint8_t)(freq>>8);
                cmd_buf[index++] = (uint8_t)(freq>>16);
                cmd_buf[index++] = (uint8_t)(freq>>24);
            }
        }
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, index);
        return CMD_OK;
    }

    i=1;
    while(i<len){
        if(cmd_argv[i] >= LORA_MAX_NB_CHANNELS){
            return ERR_PARA_INVALID;
        }
        if((len-i) < CMD_RXWIN1_LEN_FREQ){
            return ERR_PARA_INVALID;
        }
        i += (CMD_RXWIN1_LEN_FREQ+1);
    }

    i=1;
    while(i<len){
        ch = cmd_argv[i];
        freq = ((uint32_t)cmd_argv[i+1]<<0) | ((uint32_t)cmd_argv[i+2]<<8) | \
            ((uint32_t)cmd_argv[i+3]<<16) | ((uint32_t)cmd_argv[i+4]<<24);

        mibReq.Type = MIB_CUSTOM_RXWIN1;
        mibReq.Param.RxWin1.Ch = ch;
        mibReq.Param.RxWin1.Freq = freq;
        if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
            return ERR_API;
        }
        cfg_set_rxwin1_freq(ch, freq);
        i += (CMD_RXWIN1_LEN_FREQ+1);
    }
    gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
    return CMD_OK;
}

int cmd_ch(uint8_t *cmd_argv, int len)
{
    int i, index, err, ch;
    uint32_t freq;
    uint8_t dr_int;
    DrRange_t dr;
    ChannelParams_t *chlist;
    ChannelParams_t chpara;
    uint16_t *chmask;

    if(len == 1){
        mibReq.Type = MIB_CHANNELS;
        LoRaMacMibGetRequestConfirm( &mibReq );
        chlist = mibReq.Param.ChannelList;

        mibReq.Type = MIB_CHANNELS_MASK;
        LoRaMacMibGetRequestConfirm( &mibReq );
        chmask = mibReq.Param.ChannelsMask;

        for(i=0, index=0; i<LORA_MAX_NB_CHANNELS; i++){
            if( ( chmask[i/16] & (1<<(i%16)) ) == 0 ){
                continue;
            }
            freq = chlist[i].Frequency;
            dr.Value = chlist[i].DrRange.Value;
            if( 0 != freq ){
                cmd_buf[index++] = i;
                cmd_buf[index++] = (uint8_t)(freq>>0);
                cmd_buf[index++] = (uint8_t)(freq>>8);
                cmd_buf[index++] = (uint8_t)(freq>>16);
                cmd_buf[index++] = (uint8_t)(freq>>24);
                cmd_buf[index++] = dr.Value;
            }
        }
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, index);
        return CMD_OK;
    }

    i=1;
    while(i<len){
        if(cmd_argv[i] < LORA_MAX_NB_CHANNELS){
            if((len-i) < CMD_CH_LEN_FREQ_DR){
                return ERR_PARA_INVALID;
            }
            i += (CMD_CH_LEN_FREQ_DR+1);
        }else{
            return ERR_PARA_INVALID;
        }
    }

    i=1;
    while(i<len){
        ch = cmd_argv[i];
        freq = ((uint32_t)cmd_argv[i+1]<<0) | ((uint32_t)cmd_argv[i+2]<<8) | \
            ((uint32_t)cmd_argv[i+3]<<16) | ((uint32_t)cmd_argv[i+4]<<24);
        dr.Value = cmd_argv[i+5];
        if(dr.Fields.Max<dr.Fields.Min){
            dr_int = dr.Fields.Max;
            dr.Fields.Max = dr.Fields.Min;
            dr.Fields.Min = dr_int;
        }

        if(freq == 0){
           err = LoRaMacChannelRemove( ch );
        }else{
            chpara.Frequency = freq;
            chpara.DrRange.Value = dr.Value;
            chpara.Band = 0;
            err = LoRaMacChannelAdd( ch, chpara );
        }
        if(err != LORAMAC_STATUS_OK){
            return ERR_API;
        }else if(err == 0){
            if( cfg_set_ch_freq(ch, freq) < 0 ){
                return ERR_EEPROM;
            }
            if( cfg_set_ch_dr_range(ch, dr.Value) < 0){
                return ERR_EEPROM;
            }
            mibReq.Type = MIB_CHANNELS_MASK;
            LoRaMacMibGetRequestConfirm( &mibReq );
            if( cfg_set_chmsk(ch/16, mibReq.Param.ChannelsMask[ch/16]) ){
                return ERR_EEPROM;
            }
        }
        i += (CMD_CH_LEN_FREQ_DR+1);
    }

    gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
    return CMD_OK;
}

int cmd_power(uint8_t *cmd_argv, int len)
{
    if(len == 1){
        mibReq.Type = MIB_POWER_DBM;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[0] = mibReq.Param.PowerDbm;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        mibReq.Type = MIB_POWER_DBM;
        mibReq.Param.PowerDbm = cmd_argv[1];
        LoRaMacMibSetRequestConfirm( &mibReq );
        if( cfg_write(CFG_POWER, mibReq.Param.PowerDbm) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_key(uint8_t *cmd_argv, int len)
{
    int i;

    if(len>=CMD_MIN_LEN){
        /** Check if parameter is valid, start from payload */
        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_KEY_SUB_NWKSKEY:
                if((len-i) < CMD_KEY_LEN_NWKSKEY){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_KEY_LEN_NWKSKEY;
                break;
            case CMD_KEY_SUB_APPSKEY:
                if((len-i) < CMD_KEY_LEN_APPSKEY){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_KEY_LEN_APPSKEY;
                break;
            case CMD_KEY_SUB_APPKEY:
                if((len-i) < CMD_KEY_LEN_APPKEY){
                    return ERR_PARA_INVALID;
                }
                i+=CMD_KEY_LEN_APPKEY;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }

        i=1;
        while(i<len){
            switch(cmd_argv[i++]){
            case CMD_KEY_SUB_NWKSKEY:
                if((len-i) < CMD_KEY_LEN_NWKSKEY){
                    return ERR_PARA_INVALID;
                }
                mibReq.Type = MIB_NWK_SKEY;
                mibReq.Param.NwkSKey = cmd_argv+i;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
                    return ERR_API;
                }
                if( cfg_write_buf(CFG_NWKSKEY, cmd_argv+i, LW_KEY_LEN) < 0 ){
                    return ERR_EEPROM;
                }
                i+=CMD_KEY_LEN_NWKSKEY;
                break;
            case CMD_KEY_SUB_APPSKEY:
                if((len-i) < CMD_KEY_LEN_APPSKEY){
                    return ERR_PARA_INVALID;
                }
                mibReq.Type = MIB_APP_SKEY;
                mibReq.Param.AppSKey = cmd_argv+i;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
                    return ERR_API;
                }
                if( cfg_write_buf(CFG_APPSKEY, cmd_argv+i, LW_KEY_LEN) < 0 ){
                    return ERR_EEPROM;
                }

                i+=CMD_KEY_LEN_APPSKEY;
                break;
            case CMD_KEY_SUB_APPKEY:
                if((len-i) < CMD_KEY_LEN_APPKEY){
                    return ERR_PARA_INVALID;
                }
                mibReq.Type = MIB_APP_SKEY;
                mibReq.Param.AppSKey = cmd_argv+i;
                if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
                    return ERR_API;
                }
                if( cfg_write_buf(CFG_APPKEY, cmd_argv+i, LW_KEY_LEN) < 0 ){
                    return ERR_EEPROM;
                }

                i+=CMD_KEY_LEN_APPKEY;
                break;
            default:
                return ERR_PARA_INVALID;
            }
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

int cmd_class(uint8_t *cmd_argv, int len)
{
    uint8_t dt;

    if(len == 1){
        mibReq.Type = MIB_DEVICE_CLASS;
        LoRaMacMibGetRequestConfirm( &mibReq );
        cmd_buf[0] = mibReq.Param.Class;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        dt = cmd_argv[1];
        if(dt > CLASS_C){
            return ERR_PARA_INVALID;
        }

        mibReq.Type = MIB_DEVICE_CLASS;
        mibReq.Param.Class = (DeviceClass_t)dt;
        if( LoRaMacMibSetRequestConfirm( &mibReq ) !=  LORAMAC_STATUS_OK ){
            return ERR_API;
        }

        if( cfg_write(CFG_CLASS, dt) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_threshold(uint8_t *cmd_argv, int len)
{
    int16_t thresh;

    if(len == 1){
        cmd_buf[0] = (uint8_t)(LORAMAC_RSSI_THRESH>>0);
        cmd_buf[1] = (uint8_t)(LORAMAC_RSSI_THRESH>>8);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 2);
        return CMD_OK;
    }

    if(len == CMD_THRESHOLD_LEN){
        thresh = (cmd_argv[1]<<0) | (cmd_argv[2]<<8);
        if( (thresh < -105) | (thresh > -55) ){
            return ERR_PARA_INVALID;
        }
        LORAMAC_RSSI_THRESH = thresh;

        if( cfg_write(CFG_THRESHOLD, thresh) < 0 ){
            return ERR_EEPROM;
        }

        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_mode(uint8_t *cmd_argv, int len)
{
    uint8_t mode;

    if(len == 1){
        cmd_buf[0] = cfg_read(CFG_MODE);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
        return CMD_OK;
    }

    if(len == CMD_ADR_LEN){
        /** 0: ABP 1: OTAA */
        mode = cmd_argv[1];
        if(mode > 1){
            return ERR_PARA_INVALID;
        }
        if( cfg_write(CFG_MODE, mode) < 0 ){
            return ERR_EEPROM;
        }
        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_gps(uint8_t *cmd_argv, int len)
{
    int i;
    uint32_t ulong, ulat;
    float *flong, *flat;

    flong = (float *)&ulong;
    flat = (float *)&ulat;

    if(len == 1){
        i = 0;
        ulong = cfg_read(CFG_GPS_LONG);
        ulat = cfg_read(CFG_GPS_LAT);
        cmd_buf[i++] = (uint8_t)(ulong>>0);
        cmd_buf[i++] = (uint8_t)(ulong>>8);
        cmd_buf[i++] = (uint8_t)(ulong>>16);
        cmd_buf[i++] = (uint8_t)(ulong>>24);
        cmd_buf[i++] = (uint8_t)(ulat>>0);
        cmd_buf[i++] = (uint8_t)(ulat>>8);
        cmd_buf[i++] = (uint8_t)(ulat>>16);
        cmd_buf[i++] = (uint8_t)(ulat>>24);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, i);
        return CMD_OK;
    }

    if(len == 9){
        ulong = (((uint32_t)cmd_argv[4])<<24) | (((uint32_t)cmd_argv[3])<<16) | \
            (((uint32_t)cmd_argv[2])<<8) | (((uint32_t)cmd_argv[1])<<0);
        ulat = (((uint32_t)cmd_argv[8])<<24) | (((uint32_t)cmd_argv[7])<<16) | \
            (((uint32_t)cmd_argv[6])<<8) | (((uint32_t)cmd_argv[5])<<0);

        if( ( *flong > 180 ) || ( *flong < -180 ) ){
            return ERR_API;
        }
        if( ( *flat > 90 ) || ( *flat < -90 ) ){
            return ERR_API;
        }

        cfg_write(CFG_GPS_LONG, ulong);
        cfg_write(CFG_GPS_LAT, ulat);

        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);

        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_mc(uint8_t *cmd_argv, int len)
{
    int i;
    uint8_t tmp[4];

    if(len == 1){
        i = 0;
        cmd_buf[i++] = cfg_read(CFG_MC_FLAG);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, i);
        return CMD_OK;
    }

    if(len == (1+1+4+16+16+4)){
        i=2;
        cmd_mc_list.Address = (((uint32_t)cmd_argv[i+0])<<24) | (((uint32_t)cmd_argv[i+1])<<16) | \
                            (((uint32_t)cmd_argv[i+2])<<8) | (((uint32_t)cmd_argv[i+3])>>0);
        i+=4;
        memcpy1(cmd_mc_list.NwkSKey, cmd_argv+i, 16);
        i+=16;
        memcpy1(cmd_mc_list.AppSKey, cmd_argv+i, 16);
        i+=16;
        cmd_mc_list.DownLinkCounter = (((uint32_t)cmd_argv[i+0])<<24) | (((uint32_t)cmd_argv[i+1])<<16) | \
                            (((uint32_t)cmd_argv[i+2])<<8) | (((uint32_t)cmd_argv[i+3])>>0);
        cmd_mc_list.Next = NULL;

        tmp[0] = cmd_mc_list.Address>>0;
        tmp[1] = cmd_mc_list.Address>>8;
        tmp[2] = cmd_mc_list.Address>>16;
        tmp[3] = cmd_mc_list.Address>>24;
        cfg_write_buf(CFG_MC_DEVADDR, tmp, 4);
        cfg_write_buf(CFG_MC_NWKSKEY, cmd_argv+1+1+4, 16);
        cfg_write_buf(CFG_MC_APPSKEY, cmd_argv+1+1+4+16, LW_KEY_LEN);

        LoRaMacMulticastChannelLink(NULL);
        if(cmd_argv[1] == 0){
            cfg_write(CFG_MC_FLAG, 0);
        }else{
            cfg_write(CFG_MC_FLAG, 1);
            LoRaMacMulticastChannelLink(&cmd_mc_list);
        }

        gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
        return CMD_OK;
    }

    return ERR_PARA_INVALID;
}

int cmd_gpio(uint8_t *cmd_argv, int len)
{
#ifndef DISABLE_CMD_GPIO
    uint32_t pin, val;
    if(len>=2){
        switch(cmd_argv[CMD_OFT_TYPE]){
        case CMD_GPIO_SUB_DEINIT:
            if( (len-2) != CMD_GPIO_LEN_DEINIT){
                return ERR_PARA_INVALID;
            }
            ugpio_deinit();
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_GPIO_SUB_ALL:
            if( (len-2) != CMD_GPIO_LEN_ALL){
                return ERR_PARA_INVALID;
            }
            val = ((uint32_t)cmd_argv[CMD_OFT_TYPE+1]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+2]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+3]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+4]<<24);
            ugpio_output(0xFFFFFFFF, val);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_GPIO_SUB_EACH:
            if( (len-2) != CMD_GPIO_LEN_EACH){
                return ERR_PARA_INVALID;
            }
            pin = ((uint32_t)cmd_argv[CMD_OFT_TYPE+1]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+2]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+3]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+4]<<24);
            val = ((uint32_t)cmd_argv[CMD_OFT_TYPE+5]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+6]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+7]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+8]<<24);
            ugpio_output(pin, val);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        }
    }
#endif

    return ERR_PARA_INVALID;
}

int cmd_sleep(uint8_t *cmd_argv, int len)
{
    if(len == 1){
        cmd_buf[0] = 0x01;
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 1);
#ifndef USE_USB
        uartd_tx_wait();
        DelayMs(100);
        uartd_deinit(0);
#endif
        return CMD_OK;
    }
    return ERR_PARA_INVALID;
}

#ifndef RHF1S003
int cmd_test(uint8_t *cmd_argv, int len)
{
    int ret;
    app_test_rf_config_t rf_config;
    uint32_t freq, count;

    if(len == 1){
        cfg_get_rfcfg(&rf_config);
        cmd_buf[0] = cfg_read(CFG_TEST);
        cmd_buf[1] = (uint8_t)(rf_config.frf>>0);
        cmd_buf[2] = (uint8_t)(rf_config.frf>>8);
        cmd_buf[3] = (uint8_t)(rf_config.frf>>16);
        cmd_buf[4] = (uint8_t)(rf_config.frf>>24);
        cmd_buf[5] = (uint8_t)(rf_config.sf>>0);
        cmd_buf[6] = (uint8_t)(rf_config.bw>>0);
        cmd_buf[7] = (uint8_t)(rf_config.tx_pr_len>>0);
        cmd_buf[8] = (uint8_t)(rf_config.tx_pr_len>>8);
        cmd_buf[9] = (uint8_t)(rf_config.rx_pr_len>>0);
        cmd_buf[10] = (uint8_t)(rf_config.rx_pr_len>>8);
        cmd_buf[11] = (uint8_t)(rf_config.pow>>0);
        gm_tx(cmd_argv[CMD_OFT_CMD], CMD_OK, cmd_buf, 12);
        return CMD_OK;
    }

    if(len>=2){
        switch(cmd_argv[CMD_OFT_TYPE]){
        case CMD_TEST_SUB_STOP:
            if( (len-2) != CMD_TEST_LEN_STOP){
                return ERR_PARA_INVALID;
            }
            app_test_set_sta(APP_TEST_STA_IDLE, NULL);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_TXCW:
            if( (len-2) != CMD_TEST_LEN_TXCW){
                return ERR_PARA_INVALID;
            }
            app_test_set_sta(APP_TEST_STA_TXCW, NULL);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_TXCLR:
            if( (len-2) != CMD_TEST_LEN_TXCLR){
                return ERR_PARA_INVALID;
            }
            app_test_set_sta(APP_TEST_STA_TXCLORA, NULL);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_RXPKT:
            if( (len-2) != CMD_TEST_LEN_RXPKT){
                return ERR_PARA_INVALID;
            }
            app_test_set_sta(APP_TEST_STA_RXLRPKT, NULL);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_TXPKT:
            if( (len-2) < CMD_TEST_LEN_TXPKT_MIN){
                return ERR_PARA_INVALID;
            }
            app_test_set_sta(APP_TEST_STA_TXLRPKT, NULL);
            ret = app_test_tx_pkt(cmd_argv+CMD_OFT_TYPE+1, len-2);
            if( ret < 0 ){
                return ERR_API;
            }
            //gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_RSSI:
            if( (len-2) != CMD_TEST_LEN_RSSI){
                return ERR_PARA_INVALID;
            }
            freq = ((uint32_t)cmd_argv[CMD_OFT_TYPE+1]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+2]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+3]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+4]<<24);
            count = ((uint32_t)cmd_argv[CMD_OFT_TYPE+5]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+6]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+7]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+8]<<24);

            app_test_set_sta(APP_TEST_STA_RSSI, NULL);
            ret = app_test_rssi(freq, count);
            if( ret < 0 ){
                return ERR_API;
            }
            return CMD_OK;
        case CMD_TEST_SUB_RFCFG:
            if( (len-2) != CMD_TEST_LEN_RFCFG){
                return ERR_PARA_INVALID;
            }

            rf_config.cr = 1;
            rf_config.frf = ((uint32_t)cmd_argv[CMD_OFT_TYPE+1]<<0) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+2]<<8) | \
            ((uint32_t)cmd_argv[CMD_OFT_TYPE+3]<<16) | ((uint32_t)cmd_argv[CMD_OFT_TYPE+4]<<24);
            rf_config.sf = cmd_argv[CMD_OFT_TYPE+5];
            rf_config.bw = cmd_argv[CMD_OFT_TYPE+6];
            rf_config.tx_pr_len = cmd_argv[CMD_OFT_TYPE+7];
            rf_config.rx_pr_len = cmd_argv[CMD_OFT_TYPE+8];
            rf_config.pow = cmd_argv[CMD_OFT_TYPE+9];

#if  0
            sprintf((char *)cmd_buf, "F:%d,SF:%d,BW:%d,TXPR:%d,RXPR:%d,POW:%d\n", \
                    rf_config.frf, rf_config.sf, rf_config.bw, rf_config.tx_pr_len, \
                        rf_config.rx_pr_len, rf_config.pow);
            gm_putstr((char*)cmd_buf);
#endif

            if(app_test_set_rf_config(&rf_config) < 0){
                return ERR_API;
            }

            cfg_set_rfcfg(&rf_config);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        case CMD_TEST_SUB_TEST:
            if( (len-2) != CMD_TEST_LEN_TEST){
                return ERR_PARA_INVALID;
            }
            if(cmd_argv[CMD_OFT_TYPE+1] > 3){
                return ERR_PARA_INVALID;
            }
            cfg_write(CFG_TEST, cmd_argv[CMD_OFT_TYPE+1]);
            gm_ack(cmd_argv[CMD_OFT_CMD], CMD_OK);
            return CMD_OK;
        }
    }

    return ERR_PARA_INVALID;
}
#endif

int cmd_exec(uint8_t *cmd_argv, int len)
{
    int i=0;
    int8_t err;

    for(i=0; i<CMD_MAX_NUM; i++){
        if( cmd_argv[CMD_OFT_CMD] == cmd_list[i].cmd ){
            err = cmd_list[i].func(cmd_argv, len);
            if(err < 0){
                gm_ack(cmd_argv[CMD_OFT_CMD], err);
            }
            break;
        }
    }

    if( i == CMD_MAX_NUM){
        /** No command is found */
        gm_ack(CMD_ERROR, ERR_CMD_UNKNOWN);
    }
    return CMD_OK;
}
