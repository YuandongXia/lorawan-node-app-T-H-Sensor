/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "nvm-board.h"
#include "nvm.h"

void nvm_init(void)
{
    hal_nvm_init();
}

uint8_t nvm_read_byte(uint32_t addr)
{
    return hal_nvm_read_byte(addr);
}

uint16_t nvm_read_word(uint32_t addr)
{
    return hal_nvm_read_word(addr);
}

uint32_t nvm_read_dword(uint32_t addr)
{
    return hal_nvm_read_dword(addr);
}

int nvm_read_byte_buf(uint32_t addr, uint8_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        buf[i] = hal_nvm_read_byte(addr+i);
    }

    return i;
}

int nvm_read_word_buf(uint32_t addr, uint16_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        buf[i] = hal_nvm_read_byte(addr+(i<<1));
    }

    return i;
}

int nvm_read_dword_buf(uint32_t addr, uint32_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        buf[i] = hal_nvm_read_byte(addr+(i<<2));
    }

    return i;
}

int nvm_write_byte(uint32_t addr, uint8_t byte)
{
    return hal_nvm_write_byte(addr, byte, true);
}

int nvm_write_word(uint32_t addr, uint16_t word)
{
    return hal_nvm_write_word(addr, word, true);
}

int nvm_write_dword(uint32_t addr, uint32_t dword)
{
    return hal_nvm_write_dword(addr, dword, true);
}

int nvm_write_byte_buf(uint32_t addr, uint8_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        hal_nvm_write_byte(addr+(i), buf[i], true);
    }

    return i;
}

int nvm_write_word_buf(uint32_t addr, uint16_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        hal_nvm_write_byte(addr+(i<<1), buf[i], true);
    }

    return i;
}

int nvm_write_dword_buf(uint32_t addr, uint32_t *buf, int len)
{
    int i;

    for(i=0; i<len; i++){
        hal_nvm_write_byte(addr+(i<<2), buf[i], true);
    }

    return i;
}
