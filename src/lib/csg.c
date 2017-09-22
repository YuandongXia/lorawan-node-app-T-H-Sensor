#include <string.h>
#include <stdio.h>
#include "csg.h"
#include "hal-csg.h"
#include "uartd.h"

csg_frame_t csg_frame;
csg_para_t csg_para;

typedef struct{
    uint8_t len;
    uint8_t dtlen;
    const uint8_t *buf;
}csg_cmd_tab_t;

const uint8_t csg_cmd_e[] = {
    0, 0, 0, 0
};

const uint8_t csg_cmd_p[] = {
    0, 0, 3, 2
};

const uint8_t csg_cmd_v[] = {
    0, 1, 1, 2
};

const uint8_t csg_cmd_i[] = {
    0, 1, 2, 2
};

const csg_cmd_tab_t csg_cmd_tab[] = {
    { sizeof(csg_cmd_e), 4, csg_cmd_e },
    { sizeof(csg_cmd_p), 3, csg_cmd_p },
    { sizeof(csg_cmd_v), 2, csg_cmd_v },
    { sizeof(csg_cmd_i), 3, csg_cmd_i },
};

uint32_t csg_bcd2dw(uint8_t *buf, uint8_t len)
{
    uint32_t ret;
    int i;

    for(ret = 0, i = len-1; i>=0; i--){
        ret = ret*100 + 10*((buf[i]>>4) & 0x0F) + (buf[i] & 0x0F);
    }
    return ret;
}

void csg_init(void)
{
    hal_csg_init(NULL);
}

int csg_rx(uint8_t *pkt)
{
    int ret, len;

    DelayMs(100);

    while(1){
        ret = hal_csg_rx(pkt + 0, 1, CSG_TIMEOUT);
        if(ret != 0 ){
            return -1;
        }
        if( pkt[0] == CSG_PREAM){
            continue;
        }else if( pkt[0] == CSG_HDR ){
            break;
        }else{
            continue;
        }
    }

    if(hal_csg_rx(pkt+1, 9, CSG_TIMEOUT) != 0){
        return -2;
    }

    if( pkt[1+6] != CSG_HDR){
        return -3;
    }
    len = pkt[9];


    if(hal_csg_rx(pkt+10, len+2, CSG_TIMEOUT) != 0){
        return -4;
    }

    if( pkt[len+11] != CSG_END ){
        return -5;
    }

    return len+12;
}

int csg_get(csg_cmd_t cmd, csg_para_t *para)
{
    int len, dtlen;
    uint8_t buf[128];

    if(cmd >= CSG_CMD_MAX){
        return -1;
    }

    csg_frame.pream = false;
    csg_frame.addr[0] = 0xAA;
    csg_frame.addr[1] = 0xAA;
    csg_frame.addr[2] = 0xAA;
    csg_frame.addr[3] = 0xAA;
    csg_frame.addr[4] = 0xAA;
    csg_frame.addr[5] = 0xAA;
    csg_frame.ctl = 0x11;
    csg_frame.len = csg_cmd_tab[cmd].len;
    memcpy1(csg_frame.data, csg_cmd_tab[cmd].buf, csg_cmd_tab[cmd].len);

    len = csg_pack(buf, &csg_frame);

    hal_csg_tx(buf, len);

    len = csg_rx(buf);
    if(len <= 0){
        return -1;
    }

    if(csg_parse(buf, len, &csg_frame) < 0){
        return -1;
    }

    if(csg_frame.len <= csg_cmd_tab[cmd].len){
        return -1;
    }

    dtlen = csg_frame.len - csg_cmd_tab[cmd].len;
    if(dtlen != csg_cmd_tab[cmd].dtlen){
        return -1;
    }

    *(uint32_t *)para = csg_bcd2dw(csg_frame.data+csg_cmd_tab[cmd].len, dtlen);

    return 0;
}

int csg_pack(uint8_t *buf, csg_frame_t *frm)
{
    int i, j, hdr_index;
    uint8_t cs;

    i = 0;
    if(frm->pream){
        buf[i++] = 0xFE;
        buf[i++] = 0xFE;
        buf[i++] = 0xFE;
        buf[i++] = 0xFE;
    }
    hdr_index = i;
    buf[i++] = CSG_HDR;
    memcpy1(buf+i, frm->addr, 6);
    i+=6;
    buf[i++] = CSG_HDR;
    buf[i++] = frm->ctl;
    buf[i++] = frm->len;
    for(j=0; j<frm->len; j++){
        buf[i+j] = frm->data[j]+0x33;
    }
    i += frm->len;
    cs = 0;
    for(j=hdr_index; j<i; j++){
        cs += buf[j];
    }
    buf[i++] = cs;
    buf[i++] = CSG_END;

    return i;
}

int csg_parse(uint8_t *rbuf, int len, csg_frame_t *frm)
{
    int i;
    uint8_t cs;
    uint8_t *buf = NULL;

    if(rbuf[0] == CSG_PREAM){
        frm->pream = true;
        /* Remove state grid 0xFE preambles */
        for(i=0; i<len; i++){
            if(rbuf[i] != CSG_PREAM){
                buf = rbuf+i;
                len = len-i;
                break;
            }
        }
    }else{
        buf = rbuf;
        frm->pream = false;
    }

    if(buf == NULL){
        return -1;
    }

    if(buf[CSG_POS_HDR0] != CSG_HDR){
        return -1;
    }
    if(buf[CSG_POS_HDR1] != CSG_HDR){
        return -4;
    }
    if(buf[len-1] != CSG_END){
        return -5;
    }

    cs = 0;
    for(i=0; i<(len-2); i++){
        cs += buf[i];
    }
    if(cs != buf[len-2]){
        return -2;
    }

    if(len != (buf[CSG_POS_LEN] + 12)){
        return -3;
    }

    frm->ctl = buf[CSG_POS_CTL];
    frm->len = buf[CSG_POS_LEN];
    memcpy(frm->addr, buf+CSG_POS_ADDR0, 6);
    for(i=0; i<frm->len; i++){
        frm->data[i] = buf[CSG_POS_DATA+i]-0x33;
    }

    return 0;
}

void csg_test(void)
{
    csg_get(CSG_CMD_ENERGY, &csg_para);
//    csg_get(CSG_CMD_VOLTAGE, &csg_para);
//    csg_get(CSG_CMD_VOLTAGE, &csg_para);
//    csg_get(CSG_CMD_VOLTAGE, &csg_para);
}

void csg_log(csg_frame_t *frm)
{
    int i;

    printf("\n");
    printf("ADDR:\t%02X %02X %02X %02X %02X %02X\n", frm->addr[0], frm->addr[1], frm->addr[2],
           frm->addr[3], frm->addr[4], frm->addr[5]);
    printf("CTL:\t%02X\n", frm->ctl);
    printf("LEN:\t%d\n", frm->len);

    printf("DATA:\t");
    for(i=0; i<frm->len; i++){
        printf("%02X ", frm->data[i]);
    }
    printf("\n\n");
}
