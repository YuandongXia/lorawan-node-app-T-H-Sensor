/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "adc-board.h"

#define VREFINT_CAL_ADDR ((uint16_t*) ((uint32_t) 0x1FF80078))
#define VDD_CALIB ((uint16_t) (300))
#define FULL_SCALE      (4095)

#define ADC_TIMEOUT         (0x1000)

uint32_t hal_adc_init_flag = 0;

const int hal_adc_channel_tab[19] = {
    PA_0, PA_1, PA_2, PA_3, PA_4, PA_5, PA_6, PA_7,
    PB_0, PB_1, PC_0, PC_1, PC_2, PC_3, PC_4, PC_5,
    NC-1, NC, NC-2,
};

static int hal_get_channle(PinNames pin)
{
    int i;
    for(i=0; i<19; i++){
        if( pin == hal_adc_channel_tab[i] ){
            return i;
        }
    }

    return -1;
}

void hal_adc_init(adc_t *obj, PinNames Pin)
{
    uint32_t timeout;

    if( obj == NULL ){
        return;
    }

    obj->Pin = Pin;

    if(hal_adc_init_flag == 1){
        return;
    }

    hal_adc_init_flag = 1;

    /* Enable the peripheral clock of the ADC and SYSCFG */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_SYSCFGEN;

    /* Disable DAC */
    if ((ADC1->CR & ADC_CR_ADEN) != 0){
        ADC1->CR &= (uint32_t)(~ADC_CR_ADEN);
    }

    /* Calibrate */
    ADC1->CR |= ADC_CR_ADCAL;

    timeout = ADC_TIMEOUT;
    while ((ADC1->ISR & ADC_ISR_EOCAL) == 0){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    ADC1->ISR |= ADC_ISR_EOCAL;

    /* Choose clock, enable auto-off, select samping mode. */
    //ADC1->CFGR2 &= ~ADC_CFGR2_CKMODE;
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;
    ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2;
}

void hal_adc_deinit()
{
    hal_adc_init_flag = 0;
    RCC->APB2RSTR = RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;
}

uint16_t hal_adc_read(adc_t *obj)
{
    int ch;
    uint16_t adc_val;
    uint32_t timeout;

    if( obj == NULL ){
        return 0;
    }

    /* Enable the peripheral clock of the ADC and SYSCFG */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_SYSCFGEN;

    /* Stop ongoing ADC coversion */
    ADC1->CR |= ADC_CR_ADSTP;

    timeout = ADC_TIMEOUT;
    while( (ADC1->CR & ADC_CR_ADSTP) != 0 ){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    ch = hal_get_channle(obj->Pin);
    if(ch < 0){
        return 0;
    }

    ADC1->CHSELR = 1<<ch; /* (3) */
    switch(ch){
    case 17:
        ADC->CCR |= ADC_CCR_VREFEN;
        SYSCFG->CFGR3 |= SYSCFG_CFGR3_EN_VREFINT | SYSCFG_CFGR3_ENBUF_VREFINT_ADC;

        timeout = ADC_TIMEOUT;
        while ((SYSCFG->CFGR3 & SYSCFG_CFGR3_VREFINT_ADC_RDYF) == 0){
            timeout--;
            if( timeout == 0 ){
                break;
            }
        }
        break;
    case 18:
        break;
    case 19:
        break;
    default:
        break;
    }
    /* Enable the ADC */
    ADC1->CR |= ADC_CR_ADEN;
    if ((ADC1->CFGR1 &  ADC_CFGR1_AUTOFF) == 0) {
        timeout = ADC_TIMEOUT;
        while ((ADC1->ISR & ADC_ISR_ADRDY) == 0){
            timeout--;
            if( timeout == 0 ){
                break;
            }
        }
    }

    /* Performs the AD converion, start the ADC conversion */
    ADC1->CR |= ADC_CR_ADSTART;

    timeout = ADC_TIMEOUT;
    while ((ADC1->ISR & ADC_ISR_EOC) == 0){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    /* Get result */
    adc_val = ADC1->DR;

    /* Ensure that no conversion on going */
    if ((ADC1->CR & ADC_CR_ADSTART) != 0){
        ADC1->CR |= ADC_CR_ADSTP;
    }

    timeout = ADC_TIMEOUT;
    while ((ADC1->CR & ADC_CR_ADSTP) != 0){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    /* Disable ADC */
    ADC1->CR |= ADC_CR_ADDIS; /* (4) */
    timeout = ADC_TIMEOUT;
    while ((ADC1->CR & ADC_CR_ADEN) != 0){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    switch(ch){
    case 17:
        ADC->CCR &= ~ADC_CCR_VREFEN;
        SYSCFG->CFGR3 &= ~(SYSCFG_CFGR3_EN_VREFINT | SYSCFG_CFGR3_ENBUF_VREFINT_ADC);
        break;
    case 18:
        break;
    case 19:
        break;
    default:
        break;
    }

    return adc_val;
}

uint16_t hal_adc_12to16(uint16_t adc_val)
{
    // 12-bit to 16-bit conversion
    adc_val = ((adc_val << 4) & (uint16_t)0xFFF0) | ((adc_val >> 8) & (uint16_t)0x000F);
    return adc_val;
}

/* Unit 0.01V */
uint16_t hal_adc_get_vdd()
{
    adc_t adc_vref;
    uint16_t vdd, vref;

    hal_adc_init(&adc_vref, NC);
    vref = hal_adc_read(&adc_vref);

    vdd = (*VREFINT_CAL_ADDR) * VDD_CALIB / vref;

    return vdd;
}

/* Unit 0.01V */
uint16_t hal_adc_get_vol(adc_t *obj)
{
    uint16_t vdd, ch;
    uint32_t ret;

    vdd = hal_adc_get_vdd();
    ch = hal_adc_read(obj);
    ret = (uint32_t)vdd*(uint32_t)ch/FULL_SCALE;

    return ret;
}
