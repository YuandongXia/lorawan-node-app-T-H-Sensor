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

Brief: timer driver

Author: YF
*/
#include "board.h"
#include "hal-tim.h"

#define HAl_TIM_CLOCK               (32768)

static uint32_t hal_tim_cnt = 0;

void hal_tim_init(void)
{
    hal_tim_cnt = 0;

    /** Enable LPTIM1 */
    RCC->APB1ENR |= RCC_APB1ENR_LPTIM1EN;

    /** Enable LPTIM1 when sleep */
    RCC->APB1SMENR |= RCC_APB1SMENR_LPTIM1SMEN;

    /** Reset LPTIM1 */
    RCC->APB1RSTR |= RCC_APB1RSTR_LPTIM1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_LPTIM1RST;

    /** DBP enable */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_DBP;

    /** Enable LSE if LSE is not on */
    if((RCC->CSR & RCC_CSR_LSERDY)!=RCC_CSR_LSERDY){
        RCC->CSR |= RCC_CSR_LSEON;
        while((RCC->CSR & RCC_CSR_LSERDY)!=RCC_CSR_LSERDY);
    }

    /** Choose LSE as clock, 00->APB, 01->LSI 01->HSI16 11->LSE */
    RCC->CCIPR = (RCC->CCIPR&~RCC_CCIPR_LPTIM1SEL) | RCC_CCIPR_LPTIM1SEL_0 | RCC_CCIPR_LPTIM1SEL_1;

    /** Divison factor 32 */
    LPTIM1->CFGR = 0;
    LPTIM1->CFGR |= LPTIM_CFGR_PRESC_0 | LPTIM_CFGR_PRESC_2;

    /** Divison factor 1 */
    LPTIM1->CFGR = 0;
    LPTIM1->CFGR |= 0;

    /** Enable ARR */
    LPTIM1->IER |= LPTIM_IER_ARRMIE | LPTIM_IER_CMPMIE;

    /** Start LPTIM1 */
    LPTIM1->CR |= LPTIM_CR_ENABLE;

    /** Set ARR to maximum value */
    LPTIM1->ARR = 0xFFFF;
    LPTIM1->CMP = 0xFFFF;
    LPTIM1->CMP = 50;

    /** LPTIM1 free run continuously */
    LPTIM1->CR |= LPTIM_CR_CNTSTRT;

    /** Enable EXTI line */
    EXTI->IMR |= EXTI_IMR_IM29;

    /** Enable LPTIM1 interrupt and set priority */
    NVIC_EnableIRQ(LPTIM1_IRQn);
    NVIC_SetPriority(LPTIM1_IRQn,3);
}

void hal_tim_deinit(void)
{
    RCC->APB1ENR &= ~RCC_APB1ENR_PWREN;

    RCC->APB1RSTR |= RCC_APB1RSTR_LPTIM1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_LPTIM1RST;

    NVIC_DisableIRQ(LPTIM1_IRQn);
}

uint32_t hal_tim_get_tick(void)
{
    __disable_irq();
    uint32_t ret = hal_tim_cnt | LPTIM1->CNT;
    if((LPTIM1->ISR & LPTIM_ISR_ARRM) == LPTIM_ISR_ARRM){
        ret = hal_tim_cnt | LPTIM1->CNT;
        ret += 0x10000;
    }
    __enable_irq();
    return ret;
}

/** return: 1 --> time is up, 0 --> waiting */
int hal_tim_check_tick(uint32_t time)
{
    int32_t dt;

    /** already get signal no need interrupt anymore */
    LPTIM1->ICR = LPTIM_ICR_CMPMCF;

    dt = time - hal_tim_get_tick();
    if(dt < 5){
        return 1;
    }else{
        if(dt > 0xFFFF){
            dt = 0xFFFF;
        }
        LPTIM1->CMP = LPTIM1->CNT+dt;
        return 0;
    }
}

uint32_t millis(void)
{
    return (hal_tim_get_tick()*1000)/(HAl_TIM_CLOCK);
}

void LPTIM1_IRQHandler(void)
{
    if((LPTIM1->ISR & LPTIM_ISR_ARRM) == LPTIM_ISR_ARRM){
        LPTIM1->ICR = LPTIM_ICR_ARRMCF;
        hal_tim_cnt += 0x10000;
    }

    if((LPTIM1->ISR & LPTIM_ISR_CMPM) == LPTIM_ISR_CMPM){
        LPTIM1->ICR = LPTIM_ICR_CMPMCF;

    }
}

