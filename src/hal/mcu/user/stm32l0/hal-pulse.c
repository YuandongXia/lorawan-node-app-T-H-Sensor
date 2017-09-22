/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/

#include "hal-pulse.h"

static uint32_t hal_pulse_cnt;

#define PULSE_IN_PB5
//#define PULSE_IN_PC0

void hal_pulse_init()
{
    Gpio_t pulse;

    RCC->APB1ENR |= RCC_APB1ENR_LPTIM1EN;

#ifdef PULSE_IN_PB5
    GpioInit( &pulse, PB_5, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_PULL_UP, 0 );
    GPIOB->AFR[0] |= 0x2  << (5 * 4);
#elif defined PULSE_IN_PC0
    GpioInit( &pulse, PC_0, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_PULL_UP, 0 );
    GPIOC->AFR[0] |= 0x2  << (0 * 4);
#else
#warning "Pulse input pin undefined"
#endif

    /* (1) Configure LPTimer in Counter on External Input1.*/
    /* (2) Enable interrupt on Autoreload match */
    /* (3) Enable LPTimer  */
    /* (4) Set Autoreload to 4 in order to get an interrupt after 10 pulses
    because the 5 first pulses don't increment the counter */
    LPTIM1->CFGR |= LPTIM_CFGR_COUNTMODE | LPTIM_CFGR_CKSEL; /* (1)*/
    LPTIM1->IER |= LPTIM_IER_ARRMIE; /* (2) */
    LPTIM1->CR |= LPTIM_CR_ENABLE; /* (3) */
    LPTIM1->ARR = 0xFFFF; /* (4) */
    LPTIM1->CR |= LPTIM_CR_CNTSTRT; /* start the counter in continuous */

    /* Configure EXTI and NVIC for LPTIM1 */
    /* (1) Configure extended interrupt for LPTIM1 */
    /* (2) Enable Interrupt on LPTIM1 */
    /* (3) Set priority for LPTIM1 */
    EXTI->IMR |= EXTI_IMR_IM29; /* (1) */
    NVIC_EnableIRQ(LPTIM1_IRQn); /* (2) */
    NVIC_SetPriority(LPTIM1_IRQn,3); /* (3) */

    hal_pulse_cnt = 0;
}

uint32_t hal_pulse_get(void)
{
    return (hal_pulse_cnt | LPTIM1->CNT);
}

void LPTIM1_IRQHandler(void)
{
    if ((LPTIM1->ISR & LPTIM_ISR_ARRM) != 0){
        LPTIM1->ICR = LPTIM_ICR_ARRMCF;
        hal_pulse_cnt += 0x10000;
    }
}
