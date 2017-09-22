/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "nvm-board.h"
#include <stdint.h>
#include <stdbool.h>

#define FLASH_PDKEY1               ((uint32_t)0x04152637) /*!< Flash power down key1 */
#define FLASH_PDKEY2               ((uint32_t)0xFAFBFCFD) /*!< Flash power down key2: used with FLASH_PDKEY1
                                                              to unlock the RUN_PD bit in FLASH_ACR */

#define FLASH_PEKEY1               ((uint32_t)0x89ABCDEF) /*!< Flash program erase key1 */
#define FLASH_PEKEY2               ((uint32_t)0x02030405) /*!< Flash program erase key: used with FLASH_PEKEY2
                                                               to unlock the write access to the FLASH_PECR register and
                                                               data EEPROM */

#define FLASH_PRGKEY1              ((uint32_t)0x8C9DAEBF) /*!< Flash program memory key1 */
#define FLASH_PRGKEY2              ((uint32_t)0x13141516) /*!< Flash program memory key2: used with FLASH_PRGKEY2
                                                               to unlock the program memory */

#define FLASH_OPTKEY1              ((uint32_t)0xFBEAD9C8) /*!< Flash option key1 */
#define FLASH_OPTKEY2              ((uint32_t)0x24252627) /*!< Flash option key2: used with FLASH_OPTKEY1 to
                                                              unlock the write access to the option byte block */

#define FLASH_ERRs                  ( FLASH_SR_WRPERR | FLASH_SR_PGAERR | \
                                        FLASH_SR_SIZERR | FLASH_SR_OPTVERR | \
                                        FLASH_SR_RDERR | FLASH_SR_NOTZEROERR | \
                                        FLASH_SR_FWWERR )
/* init function */
void hal_nvm_init(void)
{

}

static inline void hal_nvm_lock(void)
{
    /* Lock flash */
    while ((FLASH->SR & FLASH_SR_BSY) != 0);
    FLASH->PECR |= FLASH_PECR_PELOCK;
}

static inline void hal_nvm_unlock(void)
{
    /* Unlock flash */
    while ((FLASH->SR & FLASH_SR_BSY) != 0);
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0){
        FLASH->PEKEYR = FLASH_PEKEY1;
        FLASH->PEKEYR = FLASH_PEKEY2;
    }
}

uint32_t hal_nvm_page_size(void)
{
    hal_nvm_unlock();

    hal_nvm_lock();

    return NVM_PAGE_SIZE;
}

int hal_nvm_erase_page(uint32_t addr)
{
    if( ( addr < NVM_START_ADDR ) && ( addr > NVM_END_ADDR ) ){
        return -1;
    }

    /* 4 bytes alignment */
    addr &= ~0x03;

    hal_nvm_unlock();

    FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_DATA;
    *(__IO uint32_t *)addr = (uint32_t)0;
    FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_DATA);

    hal_nvm_lock();

    return 0;
}

int hal_nvm_write_byte(uint32_t addr, uint8_t byte, bool erase)
{
    uint32_t tmp, addr_4, addr_index;

    /* skip if no need update */
    if(hal_nvm_read_byte(addr) == byte){
        return 0;
    }

    addr_4 = addr & ~0x03;
    addr_index = addr & 0x03;

    tmp = *(__IO uint32_t *)addr_4 & (~(0xFF << (addr_index << 3))) | ((uint32_t)byte << (addr_index << 3));

    hal_nvm_unlock();

    if(erase){
        while ((FLASH->SR & FLASH_SR_BSY) != 0);
        FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_DATA;
        *(__IO uint32_t *)addr_4 = (uint32_t)0;
        FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_DATA);
    }

    *(__IO uint32_t *)addr_4 = tmp;

    while ((FLASH->SR & FLASH_SR_BSY) != 0);
    // Check for Errors
    if (FLASH->SR & (FLASH_ERRs)) {
        FLASH->SR |= FLASH_ERRs;                    // clear error flags
        return (-1);                                // Failed
    }

    hal_nvm_lock();
    return 0;
}

int hal_nvm_write_word(uint32_t addr, uint16_t word, bool erase)
{
    uint32_t tmp, addr_4;

    if( 0 != (addr&0x01) ){
        return -1;
    }

    /* skip if no need update */
    if(hal_nvm_read_word(addr) == word){
        return 0;
    }

    addr_4 = ( uint32_t )( ( uint32_t ) addr & ( ~ ( uint32_t ) 0x03 ) );

    hal_nvm_unlock();

    tmp = *(__IO uint32_t *)addr_4 & (~(0xFFFF << ((addr&0x03) << 3))) | ((uint32_t)word << (((addr&0x03) << 3)));

    if(erase){
        while ((FLASH->SR & FLASH_SR_BSY) != 0);
        FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_DATA;
        *(__IO uint32_t *)addr_4 = (uint32_t)0;
        FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_DATA);
    }

    *(__IO uint32_t *)addr_4 = tmp;

    while ((FLASH->SR & FLASH_SR_BSY) != 0);
    // Check for Errors
    if (FLASH->SR & (FLASH_ERRs)) {
        FLASH->SR |= FLASH_ERRs;                    // clear error flags
        return (-1);                                // Failed
    }

    hal_nvm_lock();

    return 0;
}

int hal_nvm_write_dword(uint32_t addr, uint32_t dword, bool erase)
{
    if( 0 != (addr&0x03) ){
        return -1;
    }

    /* skip if no need update */
    if(hal_nvm_read_dword(addr) == dword){
        return 0;
    }

    hal_nvm_unlock();

    if(erase){
        while ((FLASH->SR & FLASH_SR_BSY) != 0);
        FLASH->PECR |= FLASH_PECR_ERASE | FLASH_PECR_DATA;
        *(__IO uint32_t *)addr = (uint32_t)0;
        FLASH->PECR &= ~(FLASH_PECR_ERASE | FLASH_PECR_DATA);
    }

    *(__IO uint32_t *)addr = dword;

    while ((FLASH->SR & FLASH_SR_BSY) != 0);
    // Check for Errors
    if (FLASH->SR & (FLASH_ERRs)) {
        FLASH->SR |= FLASH_ERRs;                    // clear error flags
        return (-1);                                // Failed
    }

    hal_nvm_lock();
    return 0;
}

uint8_t hal_nvm_read_byte(uint32_t addr)
{
    return *(__IO uint8_t *)addr;
}

/* 1 word = 4 bytes */
uint16_t hal_nvm_read_word(uint32_t addr)
{
    return *(__IO uint16_t *)addr;
}

uint32_t hal_nvm_read_dword(uint32_t addr)
{
    return *(__IO uint32_t *)addr;
}

