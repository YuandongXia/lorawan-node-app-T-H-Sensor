/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2015 RisingHF, all rights reserved.
*/
#include "board.h"
#include "hal-pwm.h"

#define HAL_PWM_CLK                 (1000000)
static bool pwm_start_flag;
static Gpio_t pwmctl;

void hal_pwm_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE13)) | (GPIO_MODER_MODE13_1);
    GPIOB->AFR[1] |= 0x6 << ((13 - 8) * 4);

    TIM21->PSC = (32-1);
    TIM21->ARR = HAL_PWM_CLK/1000;
    TIM21->CCR1 = HAL_PWM_CLK/500/4;
    TIM21->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
    TIM21->CCER |= TIM_CCER_CC1E;
    TIM21->CR1 |= TIM_CR1_CEN;
    TIM21->EGR |= TIM_EGR_UG;

    pwm_start_flag = true;
    GpioInit(&pwmctl, PB_14, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioWrite(&pwmctl, 1);

    hal_pwm_set_dc(0);
}

void hal_pwm_set_freq(uint32_t freq)
{

}

void hal_pwm_set_dc(double dc)
{
    uint32_t reg, arr;

    if(dc == 0){
        TIM21->CR1 &= ~TIM_CR1_CEN;
        TIM21->CCMR1 = (TIM21->CCMR1 & ~TIM_CCMR1_OC1M ) | TIM_CCMR1_OC1M_2;
        pwm_start_flag = false;
        GpioWrite(&pwmctl, 0);
    }else{
        if(pwm_start_flag == false){
            TIM21->CCMR1 = (TIM21->CCMR1 & ~TIM_CCMR1_OC1M ) | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
            TIM21->CR1 |= TIM_CR1_CEN;
            pwm_start_flag = true;
            GpioWrite(&pwmctl, 1);
        }
        arr = TIM21->ARR;
        reg = (uint32_t)(dc * arr);
        if(reg > arr){
            reg = arr;
        }
        TIM21->CCR1 = reg;
        TIM21->EGR |= TIM_EGR_UG;
    }
}
