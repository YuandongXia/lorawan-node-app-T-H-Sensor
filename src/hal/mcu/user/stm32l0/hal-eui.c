/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.

  This program and the accompanying materials are made available under the
  terms of the Eclipse Public License v1.0 which accompanies this
  distribution, and is available at
    http://www.eclipse.org/legal/epl-v10.html

Brief: EUI driver

Author: YF
*/
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hal-eui.h"
#include "utilities.h"


#define         ID0                                 ( 0x1FF80050 )
#define         ID1                                 ( 0x1FF80054 )
#define         ID2                                 ( 0x1FF80064 )

typedef union{
    uint8_t buf[12];
    struct{
        uint32_t id0;
        uint32_t id1;
        uint32_t id2;
    }dws;
}hal_eui96_t;

void hal_eui_init(void)
{
    int i;
    uint8_t tmp;
    char num[7];
    char *ptr;
    int ret;
    hal_eui96_t eui96;

    eui96.dws.id0 = *(uint32_t *)ID2;
    eui96.dws.id1 = *(uint32_t *)ID1;
    eui96.dws.id2 = *(uint32_t *)ID0;
    for(i=0; i<12/2; i++){
        tmp = eui96.buf[i];
        eui96.buf[i] = eui96.buf[11-i];
        eui96.buf[11-i] = tmp;
    }
    memcpy1((uint8_t*)num, eui96.buf+2, 6);
    num[6] = '\0';

    ret = strtol(num, &ptr, 10);
    if(*ptr != '\0'){
        while(1);
    }
    if( ret == 0 ){
        while(1);
    }
}

int hal_eui64(uint8_t *eui64)
{
    int i;
    uint8_t tmp;
    char num[7];
    char *ptr;
    int ret;
    hal_eui96_t eui96;

    eui96.dws.id0 = *(uint32_t *)ID2;
    eui96.dws.id1 = *(uint32_t *)ID1;
    eui96.dws.id2 = *(uint32_t *)ID0;
    for(i=0; i<12/2; i++){
        tmp = eui96.buf[i];
        eui96.buf[i] = eui96.buf[11-i];
        eui96.buf[11-i] = tmp;
    }
    memcpy1((uint8_t*)num, eui96.buf+2, 6);
    num[6] = '\0';

    ret = strtol(num, &ptr, 10);
    if(*ptr != '\0'){
        return -1;
    }
    if( ret == 0 ){
        return -1;
    }

    eui64[0] = eui96.buf[1];
    eui64[1] = (uint8_t)(ret>>16) ^ (eui96.buf[0]<<4) ^ (eui96.buf[0]>>4);
    eui64[2] = (uint8_t)(ret>>8);
    eui64[3] = (uint8_t)(ret>>0);

    eui64[4] = eui96.buf[8];
    eui64[5] = eui96.buf[9];
    eui64[6] = eui96.buf[10];
    eui64[7] = eui96.buf[11];

    return 0;
}
