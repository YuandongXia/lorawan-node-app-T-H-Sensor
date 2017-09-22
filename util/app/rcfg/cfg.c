#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "cfg.h"
#include "parson.h"
#include "str2hex.h"
#include "print.h"
#include "log.h"

#define CFG_PSIZE           "%20s"

const char *config_mode_tab[]={
    "ABP",
    "OTAA",
};

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

const char *config_drschm_tab[]={
    "EU868",        // 863 ~ 870        0
    "EU433",        // 433 ~ 434        1
    "US915",        // 902 ~ 928        2
    "US915HYBRID",  // 902 ~ 928        3
    "CN779",        // 779 ~ 787        4
    "AU915",        // 915 ~ 928        5
    "****",         // empty            6
    "STE920",       //                  7
    "AU915OLD",     //                  8
    "CN470",        //                  9
    "AS923",        //                  10
    "KR920",        //                  11
    "IN865",        //                  12
    "CN470PREQUEL", //                  13
    /* alias */
    "EU434",        //                  14
    "AU920",        //                  15
    "CN780",        //                  16
};

const char *config_ulfmt_tab[]={
    "DEFAULT",
    "STANDARD",
    "STE",
    "DIRECT",
};

const char *config_class_tab[]={
    "A",
    "B",
    "C",
};

static void cfg_cpy(uint8_t *distbuf, uint8_t *origbuf, int len)
{
    int i;
    for(i=0; i<len; i++){
        distbuf[i] = origbuf[len-i-1];
    }
}

int cfg_get_drschm(char *drschm)
{
    int i, max_len;
    max_len = sizeof(config_drschm_tab)/sizeof(char *);
    for(i=0; i<max_len; i++){
        if(0 == strcasecmp(drschm, config_drschm_tab[i])){
            break;
        }
    }
    if(i==max_len){
        return -1;
    }
    return i;
}

int cfg_load(const char *file, cfg_t *config)
{
    JSON_Value *jvroot;
    JSON_Object *joroot;
    JSON_Object *joch;
    JSON_Array *jarray;
    JSON_Value_Type jtype;
    const char *string;
    int i, j, max_len;

    memset(config, 0, sizeof(cfg_t));

    /* parsing json and validating output */
    jvroot = json_parse_file_with_comments(file);
    jtype = json_value_get_type(jvroot);
    if (jtype != JSONObject) {
        return -1;
    }

    joroot = json_value_get_object(jvroot);

    // Uplink format
    string = json_object_get_string(joroot, "uplink_format");
    if(string == NULL){
        config->ulfmt.flag = false;
    }else{
        max_len = sizeof(config_ulfmt_tab)/sizeof(char *);
        for(i=0; i<max_len; i++){
            if(0 == strcasecmp(string, config_ulfmt_tab[i])){
                config->ulfmt.val = i;
                config->ulfmt.flag = true;
				break;
            }
        }
    }

    // Mode
    string = json_object_get_string(joroot, "mode");
    if(string == NULL){
        config->mode.flag = false;
    }else{
        max_len = sizeof(config_mode_tab)/sizeof(char *);
        for(i=0; i<max_len; i++){
            if(0 == strcasecmp(string, config_mode_tab[i])){
                config->mode.val = i;
                config->mode.flag = true;
                break;
            }
        }
        if(i==max_len){
            config->mode.flag = false;
        }
    }

    // Datarate scheme
    jvroot = json_object_dotget_value(joroot, "dr");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->dr.val = (int)json_value_get_number(jvroot);
        config->dr.flag = true;
    }
    string = json_object_get_string(joroot, "datarate_scheme");
    if(string == NULL){
        config->drschm.flag = false;
    }else{
        max_len = sizeof(config_drschm_tab)/sizeof(char *);
        for(i=0; i<max_len; i++){
            if(0 == strcasecmp(string, config_drschm_tab[i])){
                config->drschm.val = i;
                config->drschm.flag = true;
                break;
            }
        }
        if(i==max_len){
            config->drschm.flag = false;
        }
    }

    // ID
    string = json_object_dotget_string(joroot, "id.devaddr");
    if(string != NULL){
        if(str2hex(string, config->id.devaddr.buf, LW_DEVADDR_LEN) == LW_DEVADDR_LEN){
            config->id.devaddr.flag = true;
        }
    }
    string = json_object_dotget_string(joroot, "id.deveui");
    if(string != NULL){
        if(str2hex(string, config->id.deveui.buf, LW_DEVEUI_LEN) == LW_DEVEUI_LEN){
            config->id.deveui.flag = true;
        }
    }
    string = json_object_dotget_string(joroot, "id.appeui");
    if(string != NULL){
        if(str2hex(string, config->id.appeui.buf, LW_APPEUI_LEN) == LW_APPEUI_LEN){
            config->id.appeui.flag = true;
        }
    }

    // Key
    string = json_object_dotget_string(joroot, "key.nwkskey");
    if(string != NULL){
        if(str2hex(string, config->key.nwkskey.buf, LW_NWKSKEY_LEN) == LW_NWKSKEY_LEN){
            config->key.nwkskey.flag = true;
        }
    }
    string = json_object_dotget_string(joroot, "key.appskey");
    if(string != NULL){
        if(str2hex(string, config->key.appskey.buf, LW_APPSKEY_LEN) == LW_APPSKEY_LEN){
            config->key.appskey.flag = true;
        }
    }
    string = json_object_dotget_string(joroot, "key.appkey");
    if(string != NULL){
        if(str2hex(string, config->key.appkey.buf, LW_APPKEY_LEN) == LW_APPKEY_LEN){
            config->key.appkey.flag = true;
        }
    }

    // RXWIN2
    jvroot = json_object_dotget_value(joroot, "rxwin2.freq");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->rxwin2.freq = (int)(json_value_get_number(jvroot)*1000000+0.5);
        jvroot = json_object_dotget_value(joroot, "rxwin2.dr");
        if(json_value_get_type(jvroot) == JSONNumber){
            config->rxwin2.dr = json_value_get_number(jvroot);
            config->rxwin2.flag = true;
        }
    }

    // GPS
    jvroot = json_object_dotget_value(joroot, "gps.longitude");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->gps.longitude = (float)json_value_get_number(jvroot);
        jvroot = json_object_dotget_value(joroot, "gps.latitude");
        if(json_value_get_type(jvroot) == JSONNumber){
            config->gps.latitude = (float)json_value_get_number(jvroot);
            config->gps.flag = true;
        }
    }

    // MC
    uint8_t mcflag = 0;
    string = json_object_dotget_string(joroot, "multicast.devaddr");
    if(string != NULL){
        if(str2hex(string, config->mc.val.devaddr, LW_DEVADDR_LEN) == LW_DEVADDR_LEN){
            mcflag |= 0x01;
        }
    }
    string = json_object_dotget_string(joroot, "multicast.nwkskey");
    if(string != NULL){
        if(str2hex(string, config->mc.val.nwkskey, LW_NWKSKEY_LEN) == LW_NWKSKEY_LEN){
            mcflag |= 0x02;
        }
    }
    string = json_object_dotget_string(joroot, "multicast.appskey");
    if(string != NULL){
        if(str2hex(string, config->mc.val.appskey, LW_APPSKEY_LEN) == LW_APPSKEY_LEN){
            mcflag |= 0x04;
        }
    }
    jvroot = json_object_dotget_value(joroot, "multicast.enable");
    if(json_value_get_type(jvroot) == JSONBoolean){
        config->mc.val.flag = json_value_get_boolean(jvroot);
        mcflag |= 0x08;
    }
    jvroot = json_object_dotget_value(joroot, "multicast.counter");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->mc.val.dfcnt = (uint32_t)json_value_get_number(jvroot);
        mcflag |= 0x10;
    }
    if(mcflag == 0x1F){
        config->mc.flag = true;
    }

    // ADR, DR, POWER, REPETITION(MSG), RETRY(CMSG), PORT
    jvroot = json_object_dotget_value(joroot, "adr");
    if(json_value_get_type(jvroot) == JSONBoolean){
        config->adr.val = json_value_get_boolean(jvroot);
        config->adr.flag = true;
    }
    jvroot = json_object_dotget_value(joroot, "power");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->power.val = (int)json_value_get_number(jvroot);
        config->power.flag = true;
    }
    jvroot = json_object_dotget_value(joroot, "rept");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->rept.val = (int)json_value_get_number(jvroot);
        config->rept.flag = true;
    }
    jvroot = json_object_dotget_value(joroot, "retry");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->retry.val = (int)json_value_get_number(jvroot);
        config->retry.flag = true;
    }
    jvroot = json_object_dotget_value(joroot, "port");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->port.val = (int)json_value_get_number(jvroot);
        config->port.flag = true;
    }

    jvroot = json_object_dotget_value(joroot, "period");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->period.val = (int)(1000000.0 * (double)json_value_get_number(jvroot) + 0.5);
        config->period.flag = true;
    }

    jvroot = json_object_dotget_value(joroot, "threshold");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->threshold.val = (int)json_value_get_number(jvroot);
        config->threshold.flag = true;
    }

    jvroot = json_object_dotget_value(joroot, "period_offset");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->period_offset.val = (int)(1000000.0 * (double)json_value_get_number(jvroot) + 0.5);
        config->period_offset.flag = true;
    }

    // Class
    string = json_object_dotget_string(joroot, "class");
    if(string != NULL){
        max_len = sizeof(config_class_tab)/sizeof(char *);
        for(i=0; i<max_len; i++){
            if(0 == strcasecmp(string, config_class_tab[i])){
                config->clss.val = i;
                config->clss.flag = true;
                break;
            }
        }
        if(i==max_len){
            config->clss.flag = false;
        }
    }

    // Channels
    jarray = json_object_get_array(joroot, "channels");
    if(jarray != NULL){
        int count = json_array_get_count(jarray);
        if(count > LW_CHANNELS_MAX_NUM){
            /* Warning: to many channels (>16) are defined */
        }
        for(i=0, j=0; i<count && i<LW_CHANNELS_MAX_NUM; i++, j++){
            int tmp;
            jvroot = json_array_get_value(jarray, i);
            if(json_value_get_type(jvroot)==JSONObject){
                joch = json_value_get_object(jvroot);
                if(json_value_get_type(json_object_get_value(joch, "index")) == JSONNumber){
                    tmp = (int)json_object_get_number(joch, "index");
                    if( (tmp > j) && (tmp < LW_CHANNELS_MAX_NUM) ){
                        j = tmp;
                    }
                }
                if(json_value_get_type(json_object_get_value(joch, "freq")) == JSONNumber){
                    config->channels[j].freq = (int)(1000000.0 * (double)json_object_get_number(joch, "freq") + 0.5);
                    if(json_value_get_type(json_object_get_value(joch, "dr_min")) == JSONNumber){
                        config->channels[j].dr_min = (int)json_object_get_number(joch, "dr_min");
                        if(json_value_get_type(json_object_get_value(joch, "dr_max")) == JSONNumber){
                            config->channels[j].dr_max = (int)json_object_get_number(joch, "dr_max");
                            config->chmsk[j/16] |= (1<<(j%16));
                        }
                    }
                }
            }
        }
    }

    jarray = json_object_get_array(joroot, "rxwin1");
    if(jarray != NULL){
        int count = json_array_get_count(jarray);
        if(count > LW_CHANNELS_MAX_NUM){
            /* Warning: to many channels (>16) are defined */
        }
        for(i=0, j=0; i<count && i<LW_CHANNELS_MAX_NUM; i++, j++){
            int tmp;
            jvroot = json_array_get_value(jarray, i);
            if(json_value_get_type(jvroot)==JSONObject){
                joch = json_value_get_object(jvroot);
                if(json_value_get_type(json_object_get_value(joch, "index")) == JSONNumber){
                    tmp = (int)json_object_get_number(joch, "index");
                    if( (tmp > j) && (tmp < LW_CHANNELS_MAX_NUM) ){
                        j = tmp;
                    }
                }
                if(json_value_get_type(json_object_get_value(joch, "freq")) == JSONNumber){
                    config->rxwin1[j].freq = (int)(1000000.0 * (double)json_object_get_number(joch, "freq") + 0.5);
                    config->rxwin1msk[j/16] |= (1<<(j%16));
                }
            }
        }
    }

    jvroot = json_object_dotget_value(joroot, "factor");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->factor.val = (int)json_value_get_number(jvroot);
        config->factor.flag = true;
    }

    jvroot = json_object_dotget_value(joroot, "me1initpul");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->me1initpul.val = (int)json_value_get_number(jvroot);
        config->me1initpul.flag = true;
    }

    jvroot = json_object_dotget_value(joroot, "me2initpul");
    if(json_value_get_type(jvroot) == JSONNumber){
        config->me2initpul.val = (int)json_value_get_number(jvroot);
        config->me2initpul.flag = true;
    }

    return  0;
}

void cfg_write(gm_device_t gmsp, cfg_t *config)
{
    int i;
    bool chflag, rx1flag;
    cmd_id_t id;
    cmd_dr_t dr;
    cmd_rxwin2_t rxwin2;
    cmd_rxwin1_t rxwin1;
    cmd_ch_t ch;
    cmd_period_t p;
    cmd_key_t key = {
        .flag = (CMD_FLAG_KEY_NWKSKEY|CMD_FLAG_KEY_APPSKEY|CMD_FLAG_KEY_APPKEY),
        .appkey = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
        .appskey = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
        .nwkskey = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
    };

    if(config->ulfmt.flag){
        if(0 == cmd_set_ulfmt(gmsp, config->ulfmt.val)){
            log_msg(PRIORITY_PLAIN, "Set UPLINK FORMAT successfully");
        }
    }

    if(config->mode.flag){
        if(0 == cmd_set_mode(gmsp, config->mode.val)){
            log_msg(PRIORITY_PLAIN, "Set MODE successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set MODE error");
        }
    }

    dr.flag = 0;
    if(config->drschm.flag){
        dr.flag |= CMD_FLAG_DR_SCHM;
        dr.schm = config->drschm.val;
    }
    if(config->dr.flag){
        dr.flag |= CMD_FLAG_DR_DFT_DR;
        dr.dft_dr = config->dr.val;
    }
    if(dr.flag != 0){
        if( 0 == cmd_set_dr(gmsp, &dr) ){
            log_msg(PRIORITY_PLAIN, "Set DR and DATA RATE SCHEME successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set DR or DATA RATE SCHEME error");
        }
    }

    id.flag = 0;
    if(config->id.devaddr.flag){
        id.flag |= CMD_FLAG_ID_DEVADDR;
        cfg_cpy(id.devaddr, config->id.devaddr.buf, LW_DEVADDR_LEN);
    }
    if(config->id.deveui.flag){
        id.flag |= CMD_FLAG_ID_DEVEUI;
        cfg_cpy(id.deveui, config->id.deveui.buf, LW_DEVEUI_LEN);
    }
    if(config->id.appeui.flag){
        id.flag |= CMD_FLAG_ID_APPEUI;
        cfg_cpy(id.appeui, config->id.appeui.buf, LW_APPEUI_LEN);
    }
    if(id.flag != 0){
        if(0==cmd_set_id(gmsp, &id)){
            log_msg(PRIORITY_PLAIN, "Set ID (DEVADDR/DEVEUI/APPEUI) successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set ID (DEVADDR/DEVEUI/APPEUI) error");
        }
    }

    key.flag = 0;
    if(config->key.nwkskey.flag){
        key.flag |= CMD_FLAG_KEY_NWKSKEY;
        memcpy(key.nwkskey, config->key.nwkskey.buf, LW_NWKSKEY_LEN);
    }
    if(config->key.appskey.flag){
        key.flag |= CMD_FLAG_KEY_APPSKEY;
        memcpy(key.appskey, config->key.appskey.buf, LW_APPSKEY_LEN);
    }
    if(config->key.appkey.flag){
        key.flag |= CMD_FLAG_KEY_APPKEY;
        memcpy(key.appkey, config->key.appkey.buf, LW_APPKEY_LEN);
    }
    if(key.flag!=0){
        if(0 == cmd_set_key(gmsp, &key)){
            log_msg(PRIORITY_PLAIN, "Set KEY (NWKSKEY/APPSKEY/APPKEY) successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set KEY (NWKSKEY/APPSKEY/APPKEY) error");
        }
    }

    if(config->rxwin2.flag){
        rxwin2.freq=config->rxwin2.freq;
        rxwin2.dr=config->rxwin2.dr;
        if(0 == cmd_set_rxwin2(gmsp, &rxwin2)){
            log_msg(PRIORITY_PLAIN, "Set RXWIN2 successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set RXWIN2 error");
        }
    }

    if(config->adr.flag){
        if(0 == cmd_set_adr(gmsp, config->adr.val)){
            log_msg(PRIORITY_PLAIN, "Set ADR successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set ADR error");
        }
    }

    if(config->power.flag){
        if(0 == cmd_set_power(gmsp, config->power.val)){
            log_msg(PRIORITY_PLAIN, "Set POWER successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set POWER error");
        }
    }

    if(config->rept.flag){
        if(0 == cmd_set_rept(gmsp, config->rept.val)){
            log_msg(PRIORITY_PLAIN, "Set REPT successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set REPT error");
        }
    }

    if(config->retry.flag){

    }

    if(config->port.flag){
        if(0 == cmd_set_port(gmsp, config->port.val)){
            log_msg(PRIORITY_PLAIN, "Set PORT successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set PORT error");
        }
    }

    if(config->clss.flag){
        if(0 == cmd_set_class(gmsp, config->clss.val)){
            log_msg(PRIORITY_PLAIN, "Set CLASS successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set CLASS error");
        }
    }

    if(config->mc.flag){
        cmd_mc_t mc;
        mc.flag = config->mc.val.flag;
        memcpy(mc.devaddr, config->mc.val.devaddr, 4);
        memcpy(mc.nwkskey, config->mc.val.nwkskey, 16);
        memcpy(mc.appskey, config->mc.val.appskey, 16);
        mc.dfcnt = config->mc.val.dfcnt;
        if(0 == cmd_set_mc(gmsp, &mc)){
            log_msg(PRIORITY_PLAIN, "Set MULTICAST successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set MULTICAST error");
        }
    }

    if(config->threshold.flag){
        if(0 == cmd_set_threshold(gmsp, config->threshold.val)){
            log_msg(PRIORITY_PLAIN, "Set THRESHOLD successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set THRESHOLD error");
        }
    }

    p.flag = 0;
    if(config->period.flag){
        p.flag |= CMD_FLAG_PERIOD;
        p.period = config->period.val;
    }
    if(config->period_offset.flag){
        p.flag |= CMD_FLAG_PEROFT;
        p.peroft = config->period_offset.val;
    }
    if(p.flag != 0){
        if(0 == cmd_set_period(gmsp, &p)){
            log_msg(PRIORITY_PLAIN, "Set PERIOD successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set PERIOD error");
        }
    }

    chflag = false;
    for(i=0; i<6; i++){
        ch.msk[i] = config->chmsk[i];
    }
    for(i=0; i<LW_CHANNELS_MAX_NUM; i++){
        if(config->chmsk[i/16] & (1<<(i%16))){
            chflag = true;
            ch.tab[i].freq = config->channels[i].freq;
            ch.tab[i].dr_max = config->channels[i].dr_max;
            ch.tab[i].dr_min = config->channels[i].dr_min;
        }
    }
    if(chflag){
        if(0==cmd_set_ch(gmsp, &ch)){
            log_msg(PRIORITY_PLAIN, "Set CH successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set CH error");
        }
    }

    rx1flag = false;
    for(i=0; i<6; i++){
        rxwin1.msk[i] = config->rxwin1msk[i];
    }
    for(i=0; i<LW_CHANNELS_MAX_NUM; i++){
        if(config->rxwin1msk[i/16] & (1<<(i%16))){
            rx1flag = true;
            rxwin1.freq[i] = config->rxwin1[i].freq;
        }
    }

    if(rx1flag){
        if(0==cmd_set_rxwin1(gmsp, &rxwin1)){
            log_msg(PRIORITY_PLAIN, "Set RXWIN1 successfully");
        }else{
            log_msg(PRIORITY_ERROR, "Set RXWIN1 error");
        }
    }

    if(config->factor.flag){
		if(0 == cmd_set_factor(gmsp, config->factor.val)){
			log_msg(PRIORITY_PLAIN, "Set FACTOR successfully");
		}else{
			log_msg(PRIORITY_ERROR, "Set FACTOR error");
		}
	}

	if(config->me1initpul.flag){
		if(0 == cmd_set_me1initpul(gmsp, config->me1initpul.val)){
			log_msg(PRIORITY_PLAIN, "Set METER1 INIT successfully");
		}else{
			log_msg(PRIORITY_ERROR, "Set METER1 INIT error");
		}
	}

	if(config->me2initpul.flag){
		if(0 == cmd_set_me2initpul(gmsp, config->me2initpul.val)){
			log_msg(PRIORITY_PLAIN, "Set METER2 INIT successfully");
		}else{
			log_msg(PRIORITY_ERROR, "Set METER2 INIT error");
		}
	}

	if(config->gps.flag){
        if(0 == cmd_set_gps(gmsp, config->gps.longitude, config->gps.latitude)){
			log_msg(PRIORITY_PLAIN, "Set GPS COORDINATE successfully");
		}else{
			log_msg(PRIORITY_ERROR, "Set GPS COORDINATE error");
		}
	}
}

void cfg_read(gm_device_t gmsp, cfg_t *config)
{
    int i, ret;
    cmd_id_t id;
    cmd_dr_t dr;
    cmd_rxwin2_t rxwin2;
    cmd_rxwin1_t rxwin1;
    cmd_ch_t ch;
    cmd_period_t p;
    int8_t power;
    int16_t threshold;
    uint8_t buf[8];
    char str[30];
    int16_t factor;
    int32_t me1initpul;
    int32_t me2initpul;
    float longitude;
    float latitude;

    ret = cmd_get_ulfmt(gmsp);
    if(ret>=0){
        const char *ulfmt_str;
        if(ret >= sizeof(config_ulfmt_tab)/sizeof(char *)){
            ulfmt_str = "Unknown";
        }else{
            ulfmt_str = config_ulfmt_tab[ret];
        }
        log_msg(PRIORITY_NOTICE, CFG_PSIZE "%s", "Uplink Format: ", ulfmt_str);
    }

    ret = cmd_get_mode(gmsp);
    if(ret>=0){
        printf(CFG_PSIZE "%s\n", "MODE: ", ret==0?"ABP":"OTAA");
    }

    ret = cmd_get_class(gmsp);
    if(ret>=0){
        printf(CFG_PSIZE "%c\n", "CLASS: ", 'A'+ret);
    }

    if(0==cmd_get_id(gmsp, &id)){
        printf(CFG_PSIZE, "DEVADDR: ");
        cfg_cpy(buf, id.devaddr, 4);
        puthbuf(buf, 4);
        printf("\n");

        printf(CFG_PSIZE, "DEVEUI: ");
        cfg_cpy(buf, id.deveui, 8);
        puthbuf(buf, 8);
        printf("\n");

        printf(CFG_PSIZE, "APPEUI: ");
        cfg_cpy(buf, id.appeui, 8);
        puthbuf(buf, 8);
        printf("\n");
    }

    if(0 == cmd_get_period(gmsp, &p)){
        printf(CFG_PSIZE "%ds\n", "PERIOD: ", p.period/1000000);
        printf(CFG_PSIZE "(+/-)%ds\n", "PERIOD OFFSET: ", p.peroft/1000000);
    }

    ret = cmd_get_adr(gmsp);
    switch(ret){
        case 0:
            printf(CFG_PSIZE "%s\n", "ADR: ", "OFF");
            break;
        case 1:
            printf(CFG_PSIZE "%s\n", "ADR: ", "ON");
            break;
        default:
            printf(CFG_PSIZE "%s\n", "ADR: ", "Unknown");
            break;
    }

    if( 0 == cmd_get_dr(gmsp, &dr) ){
        const char *schm;
        if(dr.schm >= sizeof(config_drschm_tab)/sizeof(char *)){
            schm = "Unknown";
        }else{
            schm = config_drschm_tab[dr.schm];
        }
        printf(CFG_PSIZE "%s\n", "DATA RATE SCHEME: ", schm);
        printf(CFG_PSIZE "DR%d\n", "DEFAULT DATA RATE: ", dr.adr_dr);
        printf(CFG_PSIZE "DR%d\n", "ADR DATA RATE: ", dr.adr_dr);
    }

    ret = cmd_get_rept(gmsp);
    if(ret>0){
        printf(CFG_PSIZE "%d\n", "REPT: ", ret);
    }

    ret = cmd_get_power(gmsp, &power);
    if(ret==0){
        printf(CFG_PSIZE "%ddBm\n", "POWER: ", power);
    }

    ret = cmd_get_port(gmsp);
    if(ret>=0){
        printf(CFG_PSIZE "%d\n", "PORT: ", ret);
    }

    ret = cmd_get_rxwin2(gmsp, &rxwin2);
    if(ret==0){
        printf(CFG_PSIZE "%d, DR%d\n", "RXWIN2: ", rxwin2.freq, rxwin2.dr);
    }

    if(0==cmd_get_ch(gmsp, &ch)){
        bool rxwin1_flag = false;
        if(0==cmd_get_rxwin1(gmsp, &rxwin1)){
            rxwin1_flag = true;
        }
        for(i=0; i<LW_CHANNELS_MAX_NUM; i++){
            if(ch.msk[i/16] & (1<<(i%16))){
                sprintf(str, "CH%d: ", i);
                if( rxwin1_flag == true ){
                    printf(CFG_PSIZE "%d, DR%d, DR%d (%d)\n", str, ch.tab[i].freq, \
                           ch.tab[i].dr_min, ch.tab[i].dr_max, rxwin1.freq[i]);
                }else{
                    printf(CFG_PSIZE "%d, DR%d, DR%d\n", str, ch.tab[i].freq, \
                           ch.tab[i].dr_min, ch.tab[i].dr_max);
                }
            }
        }
    }

    ret = cmd_get_mc(gmsp);
    if(ret>=0){
        printf(CFG_PSIZE "%s\n", "MULTICAST: ", ret == 0 ? "OFF" : "ON");
    }

    ret = cmd_get_threshold(gmsp, &threshold);
    if(ret==0){
        printf(CFG_PSIZE "%ddBm\n", "THRESHOLD: ", threshold);
    }

    ret = cmd_get_factor(gmsp, &factor);
	if(ret == 0){
		printf(CFG_PSIZE "%d \n","FACTOR: ", factor);
	}

	ret = cmd_get_me1initpul(gmsp, &me1initpul);
	if(ret == 0){
		printf(CFG_PSIZE "%d \n", "METER1 INIT: ", me1initpul);
	}

	ret = cmd_get_me2initpul(gmsp, &me2initpul);
	if(ret == 0){
		printf(CFG_PSIZE "%d \n", "METER2 INIT: ", me2initpul);
	}

	ret = cmd_get_gps(gmsp, &longitude, &latitude);
    if(ret == 0){
        printf(CFG_PSIZE "%.6f \n", "LONGTITUDE: ", longitude);
        printf(CFG_PSIZE "%.6f \n", "LATTITUDE: ", latitude);
    }
}

void cfg_puts(cfg_t *config)
{
    int i;
    char str[30];

    if(config->ulfmt.flag){
        const char *ulfmt_str;
        if(config->ulfmt.val >= sizeof(config_ulfmt_tab)/sizeof(char *)){
            ulfmt_str = "Unknown";
        }else{
            ulfmt_str = config_ulfmt_tab[config->ulfmt.val];
        }
        log_msg(PRIORITY_NOTICE, CFG_PSIZE "%s", "Uplink Format: ", ulfmt_str);
    }

    if(config->mode.flag){
        printf(CFG_PSIZE "%s\n", "MODE: ", config->mode.val==0?"ABP":"OTAA");
    }
    if(config->drschm.flag){
        const char *schm;
        if(config->drschm.val >= sizeof(config_drschm_tab)/sizeof(char *)){
            schm = "Unknown";
        }else{
            schm = config_drschm_tab[config->drschm.val];
        }
        printf(CFG_PSIZE "%s\n", "DATA RATE SCHEME: ", schm);
    }

    if(config->id.devaddr.flag){
        printf(CFG_PSIZE, "DEVADDR: ");
        puthbuf(config->id.devaddr.buf, LW_DEVADDR_LEN);
        printf("\n");
    }
    if(config->id.deveui.flag){
        printf(CFG_PSIZE, "DEVEUI: ");
        puthbuf(config->id.deveui.buf, LW_DEVEUI_LEN);
        printf("\n");
    }
    if(config->id.appeui.flag){
        printf(CFG_PSIZE, "APPEUI: ");
        puthbuf(config->id.appeui.buf, LW_APPEUI_LEN);
        printf("\n");
    }

    if(config->key.nwkskey.flag){
        printf(CFG_PSIZE, "NWKSKEY: ");
        puthbuf(config->key.nwkskey.buf, LW_NWKSKEY_LEN);
        printf("\n");
    }
    if(config->key.appskey.flag){
        printf(CFG_PSIZE, "APPSKEY: ");
        puthbuf(config->key.appskey.buf, LW_APPSKEY_LEN);
        printf("\n");
    }
    if(config->key.appkey.flag){
        printf(CFG_PSIZE, "APPKEY: ");
        puthbuf(config->key.appkey.buf, LW_APPKEY_LEN);
        printf("\n");
    }

    if(config->rxwin2.flag){
        printf(CFG_PSIZE "%d, DR%d\n", "RXWIN2: ", config->rxwin2.freq, config->rxwin2.dr);
    }

    if(config->adr.flag){
        printf(CFG_PSIZE "%s\n", "ADR: ",(config->adr.val)?"ON":"OFF");
    }
    if(config->dr.flag){
        printf(CFG_PSIZE "%d\n", "DR: ", config->dr.val);
    }
    if(config->power.flag){
        printf(CFG_PSIZE "%ddBm\n", "POWER: ", config->power.val);
    }
    if(config->rept.flag){
        printf(CFG_PSIZE "%d\n", "REPITITION: ", config->rept.val);
    }
    if(config->retry.flag){
        printf(CFG_PSIZE "%d\n", "RETRY: ", config->retry.val);
    }
    if(config->port.flag){
        printf(CFG_PSIZE "%d\n", "PORT: ", config->port.val);
    }
    if(config->clss.flag){
        printf(CFG_PSIZE "%c\n", "CLASS: ", 'A'+config->clss.val);
    }

    if(config->mc.flag){
        printf(CFG_PSIZE "%s\n", "MULTICAST: ",(config->mc.val.flag)?"ON":"OFF");
        printf(CFG_PSIZE "DEVADDR ", "MULTICAST: ");
        puthbuf(config->mc.val.devaddr, LW_DEVADDR_LEN);
        printf("\n");
        printf(CFG_PSIZE "NWKSKEY ", "MULTICAST: ");
        puthbuf(config->mc.val.nwkskey, LW_NWKSKEY_LEN);
        printf("\n");
        printf(CFG_PSIZE "APPSKEY ", "MULTICAST: ");
        puthbuf(config->mc.val.appskey, LW_NWKSKEY_LEN);
        printf("\n");
        printf(CFG_PSIZE "COUNTER %d\n", "MULTICAST: ", config->mc.val.dfcnt);
    }

    if(config->period.flag){
        printf(CFG_PSIZE "%ds\n", "PERIOD: ", config->period.val/1000000);
    }
    if(config->period_offset.flag){
        printf(CFG_PSIZE "(+/-)%ds\n", "PERIOD OFFSET: ", config->period_offset.val/1000000);
    }

    for(i=0; i<LW_CHANNELS_MAX_NUM; i++){
        if(config->chmsk[i/16] & (1<<(i%16))){
            sprintf(str, "CH%d: ", i);
            printf(CFG_PSIZE "%d, DR%d, DR%d\n",str, config->channels[i].freq, config->channels[i].dr_min, config->channels[i].dr_max);
        }
    }

    if(config->threshold.flag){
        printf(CFG_PSIZE "%ddBm\n", "THRESHOLD: ", config->threshold.val);
    }

    if(config->gps.flag){
        printf(CFG_PSIZE "%.6f \n", "LONGTITUDE: ", config->gps.longitude);
        printf(CFG_PSIZE "%.6f \n", "LATTITUDE: ", config->gps.latitude);
    }

    if(config->factor.flag){
    	printf(CFG_PSIZE "%d\n", "FACTOR: ", config->factor.val);
    }
    if(config->me1initpul.flag){
    	printf(CFG_PSIZE "%d\n", "METER1_INIT: ", config->me1initpul.val);
    }
    if(config->me2initpul.flag){
    	printf(CFG_PSIZE "%d\n", "METER2_INIT: ", config->me2initpul.val);
    }
}

