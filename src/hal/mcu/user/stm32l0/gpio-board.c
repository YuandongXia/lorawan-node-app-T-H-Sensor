/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "gpio-board.h"

static GpioIrqHandler *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
    GPIO_TypeDef *port;

    uint32_t pinnum;

    if( pin == NC )
    {
        return;
    }

    obj->portIndex = ( uint32_t ) pin >> 4;
    obj->pin = pin;
    obj->pinIndex = ( 0x01 << ( obj->pin & 0x0F ) );

    pinnum = pin & 0x0F;

    if( (obj->portIndex == 7) || (obj->portIndex < 4) )
    {
        obj->port = ( GPIO_TypeDef * )( GPIOA_BASE + ( obj->portIndex << 10 ) );
           RCC->IOPENR |= (0x01 << obj->portIndex );
    }else{
        return;
    }

    port = obj->port;
    port->MODER = ( port->MODER & ~(0x03 << (2*pinnum) ) ) | ( (mode & 0x03) << (2*pinnum) );
    port->OTYPER = ( port->OTYPER & ~(0x01 << (pinnum) ) ) | ( (config & 0x01) << (pinnum) );
    port->PUPDR = ( port->PUPDR & ~(0x03 << (2*pinnum) ) ) | ( (type & 0x03) << (2*pinnum) );
    port->OSPEEDR = ( port->OSPEEDR & ~(0x03 << (2*pinnum) ) ) | ( 0x03 << (2*pinnum) );    // High speed
}

/*
 * IRQ_VERY_LOW_PRIORITY = 0,      // 15
 * IRQ_LOW_PRIORITY,               // 12
 * IRQ_MEDIUM_PRIORITY,            // 9
 * IRQ_HIGH_PRIORITY,              // 6
 * IRQ_VERY_HIGH_PRIORITY          // 3 
 */
void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
    uint32_t syscfg_exticr_index, syscfg_exticr_exti_index;
    uint32_t pinnum;
    IRQn_Type irq_type;
    uint32_t priority;

    if( ( obj == NULL ) || ( obj->port == NULL ) )
    {
        return;
    }

    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }

    pinnum = obj->pin & 0x0F;

    GpioIrq[ pinnum ] = irqHandler;

    syscfg_exticr_index = pinnum/4;
    syscfg_exticr_exti_index = pinnum%4;

    /** enable syscfg clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Connect EXTI Line to GPIO pin */
    SYSCFG->EXTICR[syscfg_exticr_index] = (SYSCFG->EXTICR[syscfg_exticr_index] & ~(0xF << (4*syscfg_exticr_exti_index))) | \
        ((obj->portIndex) << (4*syscfg_exticr_exti_index));

    EXTI->PR = obj->pinIndex;
    if(irqMode == NO_IRQ){
        EXTI->IMR &= ~obj->pinIndex;        // Disable interrupt
        EXTI->EMR &= ~obj->pinIndex;        // Disable event
    }else{
        EXTI->IMR |= obj->pinIndex;        // Enable interrupt
        EXTI->EMR |= obj->pinIndex;        // Enable event
    }

    switch(irqMode){
    case IRQ_RISING_EDGE:
        EXTI->RTSR |= obj->pinIndex;
        EXTI->FTSR &= ~obj->pinIndex;
        break;
    case IRQ_FALLING_EDGE:
        EXTI->FTSR |= obj->pinIndex;
        EXTI->RTSR &= ~obj->pinIndex;
        break;
    case IRQ_RISING_FALLING_EDGE:
        EXTI->FTSR |= obj->pinIndex;
        EXTI->RTSR |= obj->pinIndex;
        break;
    default:
        break;
    }

    if(pinnum<2){
        irq_type = EXTI0_1_IRQn;
    }else if(pinnum<4){
        irq_type = EXTI2_3_IRQn;
    }else if(pinnum<16){
        irq_type = EXTI4_15_IRQn;
    }else{
        while(1);
    }
    
    priority = 15-3*irqPriority;
    NVIC_EnableIRQ(irq_type);
    NVIC_SetPriority(irq_type, priority);
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{

}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{
    if( ( obj == NULL ) || ( obj->port == NULL ) )
    {
        return;
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    if( value == 0 )
    {
        (( GPIO_TypeDef * )(obj->port))->BRR = obj->pinIndex;
    }
    else
    {
        (( GPIO_TypeDef * )(obj->port))->BSRR = obj->pinIndex;
    }
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
    if( ( obj == NULL ) || ( obj->port == NULL ) )
    {
        return 0;
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return 0;
    }
    if( ( (( GPIO_TypeDef * )(obj->port))->IDR & obj->pinIndex ) ){
        return 1;
    }
    return 0;
}

void EXTI0_1_IRQHandler( void )
{
#ifndef USE_NO_TIMER
    RtcRecoverMcuStatus( );
#endif
    if ((EXTI->PR & EXTI_PR_PR0) != 0)  /* Check line 0 has triggered the IT */
    {
        EXTI->PR = EXTI_PR_PR0; /* Clear the pending bit */
        if( GpioIrq[0] != NULL )
        {
            GpioIrq[0]( );
        }
    }
    if ((EXTI->PR & EXTI_PR_PR1) != 0)  /* Check line 1 has triggered the IT */
    {
        EXTI->PR = EXTI_PR_PR1; /* Clear the pending bit */
        if( GpioIrq[1] != NULL )
        {
            GpioIrq[1]( );
        }
    }
}

void EXTI2_3_IRQHandler( void )
{
#ifndef USE_NO_TIMER
    RtcRecoverMcuStatus( );
#endif
    if ((EXTI->PR & EXTI_PR_PR2) != 0)  /* Check line 2 has triggered the IT */
    {
        EXTI->PR = EXTI_PR_PR2; /* Clear the pending bit */
        if( GpioIrq[2] != NULL )
        {
            GpioIrq[2]( );
        }
    }
    if ((EXTI->PR & EXTI_PR_PR3) != 0)  /* Check line 3 has triggered the IT */
    {
        EXTI->PR = EXTI_PR_PR3; /* Clear the pending bit */
        if( GpioIrq[3] != NULL )
        {
            GpioIrq[3]( );
        }
    }
}

void EXTI4_15_IRQHandler( void )
{
#ifndef USE_NO_TIMER
    RtcRecoverMcuStatus( );
#endif
    int i;
    for(i=4; i<16; i++){
        if ((EXTI->PR & (1<<i)) != 0)  /* Check line 4~15 has triggered the IT */
        {
            EXTI->PR = (1<<i); /* Clear the pending bit */
            if( GpioIrq[i] != NULL )
            {
                GpioIrq[i]( );
            }
        }
    }
}
