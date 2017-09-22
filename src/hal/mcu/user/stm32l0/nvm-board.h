/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __NVM_BOARD_H
#define __NVM_BOARD_H

#include "board.h"

#define NVM_PAGE_SIZE           (4)
#define NVM_START_ADDR          (DATA_EEPROM_BASE)
#define NVM_END_ADDR            (DATA_EEPROM_END)

void hal_nvm_init(void);
uint32_t hal_nvm_page_size(void);
int hal_nvm_erase_page(uint32_t addr);

int hal_nvm_write_byte(uint32_t addr, uint8_t byte, bool erase);
int hal_nvm_write_word(uint32_t addr, uint16_t word, bool erase);
int hal_nvm_write_dword(uint32_t addr, uint32_t dword, bool erase);

uint8_t hal_nvm_read_byte(uint32_t addr);
uint16_t hal_nvm_read_word(uint32_t addr);
uint32_t hal_nvm_read_dword(uint32_t addr);

#endif
