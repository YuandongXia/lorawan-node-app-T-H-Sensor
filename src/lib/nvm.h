/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __NVM_H
#define __NVM_H

#include "board.h"
#include "nvm-board.h"

void nvm_init(void);

uint8_t nvm_read_byte(uint32_t addr);
uint16_t nvm_read_word(uint32_t addr);
uint32_t nvm_read_dword(uint32_t addr);
int nvm_read_byte_buf(uint32_t addr, uint8_t *buf, int len);
int nvm_read_word_buf(uint32_t addr, uint16_t *buf, int len);
int nvm_read_dword_buf(uint32_t addr, uint32_t *buf, int len);

int nvm_write_byte(uint32_t addr, uint8_t byte);
int nvm_write_word(uint32_t addr, uint16_t word);
int nvm_write_dword(uint32_t addr, uint32_t dword);
int nvm_write_byte_buf(uint32_t addr, uint8_t *buf, int len);
int nvm_write_word_buf(uint32_t addr, uint16_t *buf, int len);
int nvm_write_dword_buf(uint32_t addr, uint32_t *buf, int len);

#endif

