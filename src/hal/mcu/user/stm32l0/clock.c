/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "stm32l0xx.h"
#include "clock.h"

#define HSI_TIMEOUT_VALUE          ((uint32_t)0x5FFFF)  /* 100 ms */
#define PLL_TIMEOUT_VALUE          ((uint32_t)0x2FFFF)  /* 100 ms */
#define CLOCKSWITCH_TIMEOUT_VALUE  ((uint32_t)0x1FFFF) /* 5 s    */

void clock_init(uint32_t clock)
{
    FLASH->ACR |= FLASH_ACR_DISAB_BUF;
    FLASH->ACR |= FLASH_ACR_PRE_READ;
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    RCC->APB1ENR |= (RCC_APB1ENR_PWREN);
    while(PWR->CSR & PWR_CSR_VOSF);
    PWR->CR = (PWR->CR & ~(PWR_CR_VOS)) | PWR_CR_VOS_0;
    while(PWR->CSR & PWR_CSR_VOSF);

    RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVEN;
    while ((RCC->CR & (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) != (RCC_CR_HSIRDY |RCC_CR_HSIDIVF));

#ifdef USE_HSE
    /* Enable HSE*/
    RCC->CR &= ~(RCC_CR_HSEBYP | RCC_CR_HSEON);
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & (RCC_CR_HSERDY)) != (RCC_CR_HSERDY));
#endif

#ifdef USE_HSE
    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL | RCC_CFGR_PLLDIV);
    RCC->CFGR |= (RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV3);
    FLASH->ACR |= FLASH_ACR_LATENCY;
#else
    /** 16MHz */
    //RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL8 | RCC_CFGR_PLLDIV2;

    /** 32MHz */
    RCC->CFGR |= RCC_CFGR_PLLSRC_HSI | RCC_CFGR_PLLMUL16 | RCC_CFGR_PLLDIV2;
    FLASH->ACR |= FLASH_ACR_LATENCY;
#endif

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY)  == 0);

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL)  == 0);

    SystemCoreClockUpdate();
}

void clock_reinit(void)
{
    /* HSI for ADC */
    RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVEN;
    while ((RCC->CR & (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) != (RCC_CR_HSIRDY |RCC_CR_HSIDIVF));

#ifdef USE_HSE
    RCC->CR &= ~(RCC_CR_HSEBYP | RCC_CR_HSEON);
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & (RCC_CR_HSERDY)) != (RCC_CR_HSERDY));
#else
    RCC->CR |= RCC_CR_HSION | RCC_CR_HSIDIVEN;
    while ((RCC->CR & (RCC_CR_HSIRDY |RCC_CR_HSIDIVF)) != (RCC_CR_HSIRDY |RCC_CR_HSIDIVF));
#endif

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY)  == 0);

    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS_PLL)  == 0);
}


