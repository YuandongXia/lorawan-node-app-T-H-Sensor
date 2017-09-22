#include "gollum.h"
#include "print.h"
#include "cmd.h"

uint8_t cmd_buf[1024];
#define CMD_FDEFAULT_PASSWD             "RisingHF"
#define CMD_TIMEOUT                     (1000)      //unit ms
#define CMD_FDFT_TIMEOUT                (5000)      //unit ms

int cmd_fdefault(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_FDEFAULT, (uint8_t *)CMD_FDEFAULT_PASSWD, strlen(CMD_FDEFAULT_PASSWD));
    ret = gm_read(gmsp, cmd_buf, CMD_FDFT_TIMEOUT);
    if(ret > 0){
         if( (cmd_buf[CMD_OFT_CMD] == CMD_FDEFAULT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_ver(gm_device_t gmsp, cmd_ver_t *ver)
{
    int ret;
    gm_write(gmsp, CMD_VER, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_VER) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 5) ){
            ver->major = cmd_buf[CMD_OFT_RET+1];
            ver->minor = cmd_buf[CMD_OFT_RET+2];
            ver->patch = cmd_buf[CMD_OFT_RET+3];
            return 0;
        }
    }
    return -1;
}

int cmd_get_id(gm_device_t gmsp, cmd_id_t *id)
{
    int ret;
    gm_write(gmsp, CMD_ID, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ID) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 22)){
            id->flag = (CMD_FLAG_ID_APPEUI|CMD_FLAG_ID_DEVEUI|CMD_FLAG_ID_DEVADDR);
            memcpy(id->devaddr, cmd_buf+2, 4);
            memcpy(id->deveui, cmd_buf+2+4, 8);
            memcpy(id->appeui, cmd_buf+2+4+8, 8);
            return 0;
        }
    }
    return -1;
}

int cmd_set_id(gm_device_t gmsp, cmd_id_t *id)
{
    int index=0, ret;
    if(id->flag&CMD_FLAG_ID_DEVADDR){
        cmd_buf[index++]=CMD_ID_SUB_SET_DEVADDR;
        memcpy(cmd_buf+index, id->devaddr, 4);
        index+=4;
    }
    if(id->flag&CMD_FLAG_ID_DEVEUI){
        cmd_buf[index++]=CMD_ID_SUB_SET_DEVEUI;
        memcpy(cmd_buf+index, id->deveui, 8);
        index+=8;
    }
    if(id->flag&CMD_FLAG_ID_APPEUI){
        cmd_buf[index++]=CMD_ID_SUB_SET_APPEUI;
        memcpy(cmd_buf+index, id->appeui, 8);
        index+=8;
    }
    gm_write(gmsp, CMD_ID, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ID) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_ulfmt(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_ULFMT, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ULFMT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_ulfmt(gm_device_t gmsp, uint8_t ulfmt)
{
    int ret;
    cmd_buf[0] = ulfmt;
    gm_write(gmsp, CMD_ULFMT, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ULFMT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_adr(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_ADR, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ADR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_adr(gm_device_t gmsp, bool sta)
{
    int ret;
    if(sta){
        cmd_buf[0] = 0x01;
    }else{
        cmd_buf[0] = 0x00;
    }
    gm_write(gmsp, CMD_ADR, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ADR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_dr(gm_device_t gmsp, cmd_dr_t *dr)
{
    int ret;
    gm_write(gmsp, CMD_DR, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_DR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 5) ){
            dr->flag = (CMD_FLAG_DR_ADR_DR | CMD_FLAG_DR_DFT_DR | CMD_FLAG_DR_SCHM);
            dr->schm = cmd_buf[CMD_OFT_RET+1];
            dr->dft_dr = cmd_buf[CMD_OFT_RET+2];
            dr->adr_dr = cmd_buf[CMD_OFT_RET+3];
            return 0;
        }
    }
    return -1;
}

int cmd_set_dr(gm_device_t gmsp, cmd_dr_t *dr)
{
    int ret, index;

    index = 0;
    if(dr->flag&CMD_FLAG_DR_DFT_DR){
        cmd_buf[index++]=CMD_DR_SUB_DR;
        cmd_buf[index++]=dr->dft_dr;
    }
    if(dr->flag&CMD_FLAG_DR_SCHM){
        cmd_buf[index++]=CMD_DR_SUB_BAND;
        cmd_buf[index++]=dr->schm;
    }

    gm_write(gmsp, CMD_DR, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, 5*CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_DR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_rept(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_REPT, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_REPT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_rept(gm_device_t gmsp, uint8_t rept)
{
    int ret;

    cmd_buf[0] = rept;
    gm_write(gmsp, CMD_REPT, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_REPT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_threshold(gm_device_t gmsp, int16_t *threshold)
{
    int ret;
    int16_t thresh;

    gm_write(gmsp, CMD_THRESHOLD, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_THRESHOLD) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 4) ){
            thresh = (cmd_buf[CMD_OFT_TYPE+1]<<0) | (cmd_buf[CMD_OFT_TYPE+2]<<8);
            *threshold = thresh;
            return 0;
        }
    }
    return -1;
}

int cmd_set_threshold(gm_device_t gmsp, int16_t threshold)
{
    int ret;

    cmd_buf[0] = (uint8_t)(threshold>>0);
    cmd_buf[1] = (uint8_t)(threshold>>8);

    gm_write(gmsp, CMD_THRESHOLD, cmd_buf, 2);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_THRESHOLD) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_gps(gm_device_t gmsp, float *longitude, float *latitude)
{
    int i, ret;
    uint32_t ulong, ulat;
    float *plong, *plat;

    gm_write(gmsp, CMD_GPS, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_GPS) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 10) ){
            i = CMD_OFT_TYPE+1;
            ulong = ((uint32_t)cmd_buf[i+0]<<0) | ((uint32_t)cmd_buf[i+1]<<8) | ((uint32_t)cmd_buf[i+2]<<16) | ((uint32_t)cmd_buf[i+3]<<24);
            ulat = ((uint32_t)cmd_buf[i+4]<<0) | ((uint32_t)cmd_buf[i+5]<<8) | ((uint32_t)cmd_buf[i+6]<<16) | ((uint32_t)cmd_buf[i+7]<<24);
            plong = (float *)&ulong;
            plat = (float *)&ulat;
            *longitude = *plong;
            *latitude = *plat;
            return 0;
        }
    }
    return -1;
}

int cmd_set_gps(gm_device_t gmsp, float longitude, float latitude)
{
    int i, ret;
    uint32_t *ulong, *ulat;

    ulong = (uint32_t *)&longitude;
    ulat = (uint32_t *)&latitude;

    i = 0;
    cmd_buf[i++] = (uint8_t)(*ulong>>0);
    cmd_buf[i++] = (uint8_t)(*ulong>>8);
    cmd_buf[i++] = (uint8_t)(*ulong>>16);
    cmd_buf[i++] = (uint8_t)(*ulong>>24);
    cmd_buf[i++] = (uint8_t)(*ulat>>0);
    cmd_buf[i++] = (uint8_t)(*ulat>>8);
    cmd_buf[i++] = (uint8_t)(*ulat>>16);
    cmd_buf[i++] = (uint8_t)(*ulat>>24);
    gm_write(gmsp, CMD_GPS, cmd_buf, i);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_GPS) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}


int cmd_get_power(gm_device_t gmsp, int8_t *power)
{
    int ret;
    gm_write(gmsp, CMD_POWER, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_POWER) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            *power = (int8_t)cmd_buf[CMD_OFT_TYPE+1];
            return 0;
        }
    }
    return -1;
}

int cmd_set_power(gm_device_t gmsp, int8_t power)
{
    int ret;
    cmd_buf[0] = power;
    gm_write(gmsp, CMD_POWER, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_POWER) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_mode(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_MODE, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_MODE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_mode(gm_device_t gmsp, uint8_t mode)
{
    int ret;

    cmd_buf[0] = mode;
    gm_write(gmsp, CMD_MODE, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_MODE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_port(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_PORT, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_PORT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_port(gm_device_t gmsp, uint8_t port)
{
    int ret;

    cmd_buf[0] = port;
    gm_write(gmsp, CMD_PORT, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_PORT) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_class(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_CLASS, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_CLASS) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_class(gm_device_t gmsp, uint8_t clss)
{
    int ret;

    cmd_buf[0] = clss;
    gm_write(gmsp, CMD_CLASS, cmd_buf, 1);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_CLASS) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_rxwin2(gm_device_t gmsp, cmd_rxwin2_t *rxwin2)
{
    int ret, index;
    gm_write(gmsp, CMD_RXWIN2, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_RXWIN2) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 7) ){
            index = CMD_OFT_TYPE+1;
            rxwin2->freq = ((uint32_t)cmd_buf[index]<<0) | ((uint32_t)cmd_buf[index+1]<<8) | ((uint32_t)cmd_buf[index+2]<<16) | ((uint32_t)cmd_buf[index+3]<<24);
            rxwin2->dr = cmd_buf[index+4];
            return 0;
        }
    }
    return -1;
}

int cmd_set_rxwin2(gm_device_t gmsp, cmd_rxwin2_t *rxwin2)
{
    int ret;

    cmd_buf[0] = (uint8_t)(rxwin2->freq>>0);
    cmd_buf[1] = (uint8_t)(rxwin2->freq>>8);
    cmd_buf[2] = (uint8_t)(rxwin2->freq>>16);
    cmd_buf[3] = (uint8_t)(rxwin2->freq>>24);
    cmd_buf[4] = rxwin2->dr;
    gm_write(gmsp, CMD_RXWIN2, cmd_buf, 5);

    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_RXWIN2) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_set_key(gm_device_t gmsp, cmd_key_t *key)
{
    int index=0, ret;
    if(key->flag&CMD_FLAG_KEY_APPKEY){
        cmd_buf[index++]=CMD_KEY_SUB_APPKEY;
        memcpy(cmd_buf+index, key->appkey, CMD_KEY_LEN_APPKEY);
        index+=CMD_KEY_LEN_APPKEY;
    }
    if(key->flag&CMD_FLAG_KEY_APPSKEY){
        cmd_buf[index++]=CMD_KEY_SUB_APPSKEY;
        memcpy(cmd_buf+index, key->appskey, CMD_KEY_LEN_APPSKEY);
        index+=CMD_KEY_LEN_APPSKEY;
    }
    if(key->flag&CMD_FLAG_KEY_NWKSKEY){
        cmd_buf[index++]=CMD_KEY_SUB_NWKSKEY;
        memcpy(cmd_buf+index, key->nwkskey, CMD_KEY_LEN_NWKSKEY);
        index+=CMD_KEY_LEN_NWKSKEY;
    }

    gm_write(gmsp, CMD_KEY, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_KEY) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_mc(gm_device_t gmsp)
{
    int ret;
    gm_write(gmsp, CMD_MC, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_MC) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 3) ){
            return cmd_buf[CMD_OFT_TYPE+1];
        }
    }
    return -1;
}

int cmd_set_mc(gm_device_t gmsp, cmd_mc_t *mc)
{
    int index=0, ret;

    cmd_buf[index++]=mc->flag;
    memcpy(cmd_buf+index, mc->devaddr, 4);
    index+=4;
    memcpy(cmd_buf+index, mc->nwkskey, 16);
    index+=16;
    memcpy(cmd_buf+index, mc->appskey, 16);
    index+=16;
    cmd_buf[index++] = (uint8_t)(mc->dfcnt>>0);
    cmd_buf[index++] = (uint8_t)(mc->dfcnt>>8);
    cmd_buf[index++] = (uint8_t)(mc->dfcnt>>16);
    cmd_buf[index++] = (uint8_t)(mc->dfcnt>>24);
    gm_write(gmsp, CMD_MC, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_MC) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_rxwin1(gm_device_t gmsp, cmd_rxwin1_t *rxwin1)
{
    int ret, index;

    memset((uint8_t *)rxwin1, 0, sizeof(cmd_rxwin1_t));

    gm_write(gmsp, CMD_RXWIN1, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, 3*CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_RXWIN1) && (cmd_buf[CMD_OFT_RET] == CMD_OK) ){
            index = CMD_OFT_RET+1;
            while(index<ret){
                if(cmd_buf[index++]<LW_CHANNELS_MAX_NUM){
                    if((ret-index) < 4){
                        return -1;
                    }
                    index += 4;
                }else{
                    return -1;
                }
            }

            index = CMD_OFT_RET+1;
            while(index<ret){
                rxwin1->freq[cmd_buf[index]] = ((uint32_t)cmd_buf[index+1]<<0) | \
                                                    ((uint32_t)cmd_buf[index+2]<<8) | \
                                                    ((uint32_t)cmd_buf[index+3]<<16) | \
                                                    ((uint32_t)cmd_buf[index+4]<<24);
                index+=5;
            }
            return 0;
        }
    }
    return -1;
}

int cmd_set_rxwin1(gm_device_t gmsp, cmd_rxwin1_t *rxwin1)
{
    int i, ret, index;
    int cmd = CMD_RXWIN1;

    for(i=0, index=0; i<LW_CHANNELS_MAX_NUM; i++){
        if(rxwin1->msk[i/16]&(1<<(i%16))){
            cmd_buf[index++] = i;
            cmd_buf[index++] = (uint8_t)(rxwin1->freq[i]>>0);
            cmd_buf[index++] = (uint8_t)(rxwin1->freq[i]>>8);
            cmd_buf[index++] = (uint8_t)(rxwin1->freq[i]>>16);
            cmd_buf[index++] = (uint8_t)(rxwin1->freq[i]>>24);
        }
    }

    gm_write(gmsp, cmd, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == cmd) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }

    return -1;
}

int cmd_get_ch(gm_device_t gmsp, cmd_ch_t *ch)
{
    int ret, index;

    memset((uint8_t *)ch, 0, sizeof(cmd_ch_t));

    gm_write(gmsp, CMD_CH, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, 10*CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_CH) && (cmd_buf[CMD_OFT_RET] == CMD_OK) ){
            index = CMD_OFT_RET+1;
            while(index<ret){
                if(cmd_buf[index++]<LW_CHANNELS_MAX_NUM){
                    if((ret-index) < 5){
                        return -1;
                    }
                    index += 5;
                }else{
                    return -1;
                }
            }

            index = CMD_OFT_RET+1;
            while(index<ret){
                ch->tab[cmd_buf[index]].freq = ((uint32_t)cmd_buf[index+1]<<0) | ((uint32_t)cmd_buf[index+2]<<8) | ((uint32_t)cmd_buf[index+3]<<16) | ((uint32_t)cmd_buf[index+4]<<24);
                ch->tab[cmd_buf[index]].dr_min = (cmd_buf[index+5]&0x0F);
                ch->tab[cmd_buf[index]].dr_max = (cmd_buf[index+5]>>4);
                if(ch->tab[cmd_buf[index]].dr_min > ch->tab[cmd_buf[index]].dr_max){
                    uint8_t tmp;
                    tmp = ch->tab[cmd_buf[index]].dr_max;
                    ch->tab[cmd_buf[index]].dr_max =  ch->tab[cmd_buf[index]].dr_min;
                    ch->tab[cmd_buf[index]].dr_min = tmp;
                }

                ch->msk[cmd_buf[index]/16] |= (1<<(cmd_buf[index]%16));
                index+=6;
                ch->num++;
            }
            return 0;
        }
    }
    return -1;
}

int cmd_set_ch(gm_device_t gmsp, cmd_ch_t *ch)
{
    int i, ret, index;

    for(i=0, index=0; i<LW_CHANNELS_MAX_NUM; i++){
        if(ch->msk[i/16]&(1<<(i%16))){
            cmd_buf[index++] = i;
            cmd_buf[index++] = (uint8_t)(ch->tab[i].freq>>0);
            cmd_buf[index++] = (uint8_t)(ch->tab[i].freq>>8);
            cmd_buf[index++] = (uint8_t)(ch->tab[i].freq>>16);
            cmd_buf[index++] = (uint8_t)(ch->tab[i].freq>>24);
            cmd_buf[index++] = (ch->tab[i].dr_max<<4) | (ch->tab[i].dr_min&0x0F);
        }
    }

    gm_write(gmsp, CMD_CH, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, 3*CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_CH) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }

    return -1;
}

int cmd_set_period(gm_device_t gmsp, cmd_period_t *period)
{
    int index=0, ret;
    if(period->flag&CMD_FLAG_PERIOD){
        cmd_buf[index++]=CMD_PERIOD_SUB_PER;
        cmd_buf[index++] = (uint8_t)(period->period>>0);
        cmd_buf[index++] = (uint8_t)(period->period>>8);
        cmd_buf[index++] = (uint8_t)(period->period>>16);
        cmd_buf[index++] = (uint8_t)(period->period>>24);
    }
    if(period->flag&CMD_FLAG_PEROFT){
        cmd_buf[index++]=CMD_PERIOD_SUB_OFT;
        cmd_buf[index++] = (uint8_t)(period->peroft>>0);
        cmd_buf[index++] = (uint8_t)(period->peroft>>8);
        cmd_buf[index++] = (uint8_t)(period->peroft>>16);
        cmd_buf[index++] = (uint8_t)(period->peroft>>24);
    }

    gm_write(gmsp, CMD_PERIOD, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_PERIOD) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_period(gm_device_t gmsp, cmd_period_t *period)
{
    int ret, i;
    gm_write(gmsp, CMD_PERIOD, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_PERIOD) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 10)){
            i=CMD_OFT_RET;
            period->flag = (CMD_FLAG_PERIOD|CMD_FLAG_PEROFT);
            period->period = (((uint32_t)cmd_buf[i+4])<<24) | (((uint32_t)cmd_buf[i+3])<<16) | \
            (((uint32_t)cmd_buf[i+2])<<8) | (((uint32_t)cmd_buf[i+1])<<0);
            i=CMD_OFT_RET+4;
            period->peroft = (((uint32_t)cmd_buf[i+4])<<24) | (((uint32_t)cmd_buf[i+3])<<16) | \
            (((uint32_t)cmd_buf[i+2])<<8) | (((uint32_t)cmd_buf[i+1])<<0);
            return 0;
        }
    }
    return -1;
}

int cmd_set_test(gm_device_t gmsp, int mode, cmd_rf_t *rfcfg)
{
    int index=0, ret;

    if(rfcfg != NULL){
        cmd_buf[index++] = CMD_TEST_SUB_RFCFG;
        cmd_buf[index++] = (uint8_t)(rfcfg->frf>>0);
        cmd_buf[index++] = (uint8_t)(rfcfg->frf>>8);
        cmd_buf[index++] = (uint8_t)(rfcfg->frf>>16);
        cmd_buf[index++] = (uint8_t)(rfcfg->frf>>24);
        cmd_buf[index++] = (uint8_t)(rfcfg->sf>>0);
        cmd_buf[index++] = (uint8_t)(rfcfg->bw>>0);
        cmd_buf[index++] = (uint8_t)(rfcfg->tx_pr_len>>0);
        //cmd_buf[index++] = (uint8_t)(rfcfg->tx_pr_len>>8);
        cmd_buf[index++] = (uint8_t)(rfcfg->rx_pr_len>>0);
        //cmd_buf[index++] = (uint8_t)(rfcfg->rx_pr_len>>8);
        cmd_buf[index++] = (uint8_t)(rfcfg->pow>>0);
        gm_write(gmsp, CMD_TEST, cmd_buf, index);
        ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
        if(ret > 0){
            if( (cmd_buf[CMD_OFT_CMD] == CMD_TEST) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){

            }
        }
    }

    index = 0;
    cmd_buf[index++] = CMD_TEST_SUB_TEST;
    cmd_buf[index++] = (uint8_t)mode;
    gm_write(gmsp, CMD_TEST, cmd_buf, index);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_TEST) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2)){
            return 0;
        }
    }
    return -1;
}

int cmd_get_test(gm_device_t gmsp, int *mode, cmd_rf_t *rfcfg)
{
    int ret, i;
    gm_write(gmsp, CMD_TEST, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_TEST) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 14)){
            i=CMD_OFT_RET;
            *mode = cmd_buf[i+1];
            i++;

            rfcfg->frf = (((uint32_t)cmd_buf[i+4])<<24) | (((uint32_t)cmd_buf[i+3])<<16) | \
            (((uint32_t)cmd_buf[i+2])<<8) | (((uint32_t)cmd_buf[i+1])<<0);
            i+=4;

            rfcfg->sf = cmd_buf[i+1];
            i++;
            rfcfg->bw = cmd_buf[i+1];
            i++;
            rfcfg->tx_pr_len = (((uint16_t)cmd_buf[i+2])<<8) | (((uint16_t)cmd_buf[i+1])<<0);
            i+=2;
            rfcfg->rx_pr_len = (((uint16_t)cmd_buf[i+2])<<8) | (((uint16_t)cmd_buf[i+1])<<0);
            i+=2;
            rfcfg->pow = cmd_buf[i+1];
            return 0;
        }
    }
    return -1;
}

int cmd_get_factor(gm_device_t gmsp, int16_t *factor)
{
    int ret;
    gm_write(gmsp, CMD_METER_FACTOR, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_METER_FACTOR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 4) ){
            *factor = (((uint16_t)cmd_buf[CMD_OFT_TYPE+2])<<8) | (((uint16_t)cmd_buf[CMD_OFT_TYPE+1])<<0);
            return 0;
        }
    }
    return -1;
}

int cmd_set_factor(gm_device_t gmsp, int16_t factor)
{
    int ret;
    cmd_buf[0] = (uint8_t)(factor>>0);
    cmd_buf[1] = (uint8_t)(factor>>8);
    gm_write(gmsp, CMD_METER_FACTOR, cmd_buf, 2);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_METER_FACTOR) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_me1initpul(gm_device_t gmsp, int32_t *me1initpul)
{
    int ret;
    gm_write(gmsp, CMD_ME1_INIT_PULSE, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ME1_INIT_PULSE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 6) ){
            *me1initpul = (((uint32_t)cmd_buf[CMD_OFT_RET+4])<<24) | (((uint32_t)cmd_buf[CMD_OFT_RET+3])<<16) | \
                          (((uint32_t)cmd_buf[CMD_OFT_RET+2])<<8) | (((uint32_t)cmd_buf[CMD_OFT_RET+1])<<0);
            return 0;
        }
    }
    return -1;
}

int cmd_set_me1initpul(gm_device_t gmsp, int32_t me1initpul)
{
    int ret;
    cmd_buf[0] = (uint8_t)(me1initpul>>0);
    cmd_buf[1] = (uint8_t)(me1initpul>>8);
    cmd_buf[2] = (uint8_t)(me1initpul>>16);
    cmd_buf[3] = (uint8_t)(me1initpul>>24);
    gm_write(gmsp, CMD_ME1_INIT_PULSE, cmd_buf, 4);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ME1_INIT_PULSE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}

int cmd_get_me2initpul(gm_device_t gmsp, int32_t *me2initpul)
{
    int ret;
    gm_write(gmsp, CMD_ME2_INIT_PULSE, NULL, 0);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ME2_INIT_PULSE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 6) ){
            *me2initpul = (((uint32_t)cmd_buf[CMD_OFT_RET+4])<<24) | (((uint32_t)cmd_buf[CMD_OFT_RET+3])<<16) | \
                          (((uint32_t)cmd_buf[CMD_OFT_RET+2])<<8) | (((uint32_t)cmd_buf[CMD_OFT_RET+1])<<0);
            return 0;
        }
    }
    return -1;
}

int cmd_set_me2initpul(gm_device_t gmsp, int32_t me2initpul)
{
    int ret;
    cmd_buf[0] = (uint8_t)(me2initpul>>0);
    cmd_buf[1] = (uint8_t)(me2initpul>>8);
    cmd_buf[2] = (uint8_t)(me2initpul>>16);
    cmd_buf[3] = (uint8_t)(me2initpul>>24);
    gm_write(gmsp, CMD_ME2_INIT_PULSE, cmd_buf, 4);
    ret = gm_read(gmsp, cmd_buf, CMD_TIMEOUT);
    if(ret > 0){
        if( (cmd_buf[CMD_OFT_CMD] == CMD_ME2_INIT_PULSE) && (cmd_buf[CMD_OFT_RET] == CMD_OK) && (ret == 2) ){
            return 0;
        }
    }
    return -1;
}
