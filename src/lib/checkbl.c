/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/

//#pragma location=(0x0800FFFF-32+1)
//__root const char bootloader_magic_number[32]="nU-4#e!ruka*edRa6aC_epr7bu8rUc#";

#include <stdint.h>
#include <string.h>
#include "cmac.h"
#include "utilities.h"

#ifdef BOOTLOADER_12K
#define CBL_KEY_ADDR                (0x08003000-16)
#elif defined (BOOTLOADER_8K)
#define CBL_KEY_ADDR                (0x08002000-16)
#else
#define CBL_KEY_ADDR                (0x08010000-16)
#endif

const uint8_t cbl_block[16] = "risinghfnU-4#e!r";
const uint8_t cbl_data[16]  = "b-eTH!H6tr-3ACra";
const uint8_t cbl_ret[16] ={
    0x81, 0x9A, 0x73, 0x79, 0xBF, 0x61, 0x2E, 0x14,
    0x2B, 0x26, 0x9B, 0x34, 0x6A, 0x0F, 0x99, 0xFF,
};

int cbl(void)
{
    AES_CMAC_CTX cmacctx;
    uint8_t *key;
    uint8_t out[16];

    memset1(out, 0, 16);

    key = (uint8_t *)CBL_KEY_ADDR;

    AES_CMAC_Init(&cmacctx);
    AES_CMAC_SetKey(&cmacctx, key);

    AES_CMAC_Update(&cmacctx, cbl_block, 16);
    AES_CMAC_Update(&cmacctx, cbl_data, 16);

    AES_CMAC_Final(out, &cmacctx);

    if( 0 != memcmp(cbl_ret, out, 16) ){
        return -1;
    }

    return 0;
}
