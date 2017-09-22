/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "i2c-board.h"

i2c_t i2c1;

/*!
 *  The value of the maximal timeout for I2C waiting loops
 */
#define TIMEOUT_MAX                                 0x8000

/* Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted. */
#define FLAG_TIMEOUT ((int)0x1000)
#define LONG_TIMEOUT ((int)0x8000)

#define I2C_CR2_MASK        (I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | \
                             I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | \
                             I2C_CR2_STOP))

int hal_i2c_init(i2c_t *obj, PinNames scl, PinNames sda)
{
    uint8_t i2c_type = 0;

    /* GPIO clock will be enabled after gpio initial funciton is called */
    GpioInit( &obj->Scl, scl, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_PULL_UP, 0 );
    GpioInit( &obj->Sda, sda, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_PULL_UP, 0 );

    /* Check whether I2C1 or I2C2 should be used,  enable alternative function */
    switch(scl){
    case PB_6:  /* AF1 */
        i2c_type |= 0x01;   // i2c1 flag
        GPIOB->AFR[0] = (GPIOB->AFR[0] &~ (0xF << (6 * 4))) | (1 << (6 * 4));
        break;
    case PB_8:  /* AF4 */
        i2c_type |= 0x01;   // i2c1 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((8-8) * 4))) | (4 << ((8-8) * 4));
        break;
    case PB_10: /* AF6 */
        i2c_type |= 0x04;   // i2c2 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((10-8) * 4))) | (6 << ((10-8) * 4));
        break;
    case PB_13: /* AF5 */
        i2c_type |= 0x04;   // i2c2 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((13-8) * 4))) | (5 << ((13-8) * 4));
        break;
    default:
        /* I2C pin is not supported */
        while(1);
    }

    switch(sda){
    case PB_7:  /* AF1 */
        i2c_type |= 0x02;   // i2c1 flag
        GPIOB->AFR[0] = (GPIOB->AFR[0] &~ (0xF << (7 * 4))) | (1 << (7 * 4));
        break;
    case PB_9:  /* AF4 */
        i2c_type |= 0x02;   // i2c1 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((9-8) * 4))) | (4 << ((9-8) * 4));
        break;
    case PB_11: /* AF6 */
        i2c_type |= 0x08;   // i2c2 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((11-8) * 4))) | (6 << ((11-8) * 4));
        break;
    case PB_14: /* AF5 */
        i2c_type |= 0x08;   // i2c2 flag
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((14-8) * 4))) | (5 << ((14-8) * 4));
        break;
    default:
        /* I2C pin is not supported */
        while(1);
    }

    if( i2c_type == 0x03 ){
        /* I2C1 */
        obj->I2c = ( hal_i2c_t * )I2C1_BASE;

        /* Enable clock */
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

        /* Choose clock source for I2C1 */
        RCC->CCIPR &= ~RCC_CCIPR_I2C1SEL;
    }else if( i2c_type == 0x0C ){
        /* I2C2 */
        obj->I2c = ( hal_i2c_t * )I2C2_BASE;
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    }else{
        /* scl and sda pin is not matched */
        while(1);
    }

    hal_i2c_reset(obj);

    return 0;
}

void hal_i2c_deinit(i2c_t *obj, PinNames scl, PinNames sda)
{
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    /* Check whether I2C1 or I2C2 should be used,  enable alternative function */
    switch(scl){
    case PB_6:  /* AF1 */
        GPIOB->AFR[0] = (GPIOB->AFR[0] &~ (0xF << (6 * 4))) | (0 << (6 * 4));
        break;
    case PB_8:  /* AF4 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((8-8) * 4))) | (0 << ((8-8) * 4));
        break;
    case PB_10: /* AF6 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((10-8) * 4))) | (0 << ((10-8) * 4));
        break;
    case PB_13: /* AF5 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((13-8) * 4))) | (0 << ((13-8) * 4));
        break;
    default:
        /* I2C pin is not supported */
        while(1);
    }

    switch(sda){
    case PB_7:  /* AF1 */
        GPIOB->AFR[0] = (GPIOB->AFR[0] &~ (0xF << (7 * 4))) | (0 << (7 * 4));
        break;
    case PB_9:  /* AF4 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((9-8) * 4))) | (0 << ((9-8) * 4));
        break;
    case PB_11: /* AF6 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((11-8) * 4))) | (0 << ((11-8) * 4));
        break;
    case PB_14: /* AF5 */
        GPIOB->AFR[1] = (GPIOB->AFR[1] &~ (0xF << ((14-8) * 4))) | (0 << ((14-8) * 4));
        break;
    default:
        /* I2C pin is not supported */
        while(1);
    }

    GpioInit( &obj->Scl, scl, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Sda, sda, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    /** Disable I2c */
    i2c_p->CR1 &=  ~I2C_CR1_PE ;

    /* Enable clock */
    RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
}

int hal_i2c_reset(i2c_t *obj)
{
    uint32_t timeout;
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    if(i2c_p == NULL){
        while(1);
        //return;
    }

    timeout = LONG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_BUSY) != 0 ){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    if( i2c_p == (( hal_i2c_t * )I2C1_BASE) ){
        RCC->APB1RSTR |= (RCC_APB1RSTR_I2C1RST);
        RCC->APB1RSTR &= ~ (RCC_APB1RSTR_I2C1RST);
    }else if( i2c_p == (( hal_i2c_t * )I2C2_BASE)  ){
        RCC->APB1RSTR |= (RCC_APB1RSTR_I2C2RST);
        RCC->APB1RSTR &= ~ (RCC_APB1RSTR_I2C2RST);
    }

    return 0;
}

void hal_i2c_frequency(i2c_t *obj, int hz)
{
    uint32_t timeout;
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    if(i2c_p == NULL){
        while(1);
        //return;
    }

    timeout = LONG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_BUSY) != 0 ){
        timeout--;
        if( timeout == 0 ){
            break;
        }
    }

    /** Disable I2c */
    i2c_p->CR1 &=  ~I2C_CR1_PE ;

    // Common settings: I2C clock = 32 MHz, Analog filter = ON, Digital filter coefficient = 0
    switch (hz) {
    case 100000:
        i2c_p->TIMINGR = 0x20602938; // 32MHz Standard mode with Rise Time = 400ns and Fall Time = 100ns
        break;
    case 1000000:
        i2c_p->TIMINGR = 0x0030040E; // 32MHz Fast mode Plus with Rise Time = 60ns and Fall Time = 100ns
        break;
    default:
    case 400000:
        //i2c_p->TIMINGR = 0x00B0122A;  // 32MHz Fast mode with Rise Time = 250ns and Fall Time = 100ns
        //i2c_p->TIMINGR = 0x00300619;    // 16MHz Fast mode with Rise Time = 10ns and Fall Time = 100ns
        i2c_p->TIMINGR = 0x0010061A;
        break;
        break;
    }

    /* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process */
    i2c_p->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

    /** Enable I2c */
    i2c_p->CR1 |=  I2C_CR1_PE ;
}

int hal_i2c_start(i2c_t *obj)
{
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);
    int timeout;

    // Clear Acknowledge failure flag
    i2c_p->ICR = I2C_ICR_NACKCF;

    // Generate the START condition
    i2c_p->CR2 |= I2C_CR2_START;

    // Wait the START condition has been correctly sent
    timeout = FLAG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_BUSY) != 0 ){
        timeout--;
        if( timeout == 0 ){
            return -1;
        }
    }
    return 0;
}

int hal_i2c_stop(i2c_t *obj)
{
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    // Generate the STOP condition
    i2c_p->CR2 |= I2C_CR2_STOP;

    return 0;
}

int hal_i2c_read(i2c_t *obj, int address, char *data, int length, int stop)
{
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);
    int timeout;
    int count;
    int value;

    /* update CR2 register */
    i2c_p->CR2 = (i2c_p->CR2 & (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP)))
        | (uint32_t)(((uint32_t)address & I2C_CR2_SADD) | (((uint32_t)length << 16) & I2C_CR2_NBYTES) | I2C_CR2_START | I2C_CR2_RD_WRN);

    // Read all bytes
    for (count = 0; count < length; count++) {
        value = hal_i2c_byte_read(obj, 0);
        data[count] = (char)value;
    }

    // Wait transfer complete
    timeout = FLAG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_TC) == 0 ){
        timeout--;
        if (timeout == 0) {
            return -1;
        }
    }

    i2c_p->ICR |= I2C_ISR_TC;

    // If not repeated start, send stop.
    if (stop) {
        hal_i2c_stop(obj);
        /* Wait until STOPF flag is set */
        timeout = FLAG_TIMEOUT;
        while( (i2c_p->ISR & I2C_ISR_STOPF) == 0 ){
            timeout--;
            if (timeout == 0) {
                return -1;
            }
        }
        /* Clear STOP Flag */
        i2c_p->ICR |= I2C_ISR_STOPF;
    }

    return length;
}

int hal_i2c_write(i2c_t *obj, int address, const char *data, int length, int stop)
{
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);
    int timeout;
    int count;

    /* update CR2 register */
    i2c_p->CR2 = (i2c_p->CR2 & (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP)))
               | (uint32_t)(((uint32_t)address & I2C_CR2_SADD) | (((uint32_t)length << 16) & I2C_CR2_NBYTES) | I2C_CR2_START);

    for (count = 0; count < length; count++) {
        if( hal_i2c_byte_write(obj, data[count]) < 0){
            return -1;
        }
    }

    // Wait transfer complete
    timeout = FLAG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_TC) == 0 ){
        timeout--;
        if (timeout == 0) {
            return -1;
        }
    }
    i2c_p->ICR |= I2C_ISR_TC;

    // If not repeated start, send stop.
    if (stop) {
        hal_i2c_stop(obj);
        /* Wait until STOPF flag is set */
        timeout = FLAG_TIMEOUT;
        while( (i2c_p->ISR & I2C_ISR_STOPF) == 0 ){
            timeout--;
            if (timeout == 0) {
                return -1;
            }
        }
        /* Clear STOP Flag */
        i2c_p->ICR |= I2C_ISR_STOPF;
    }

    return count;
}

int hal_i2c_byte_read(i2c_t *obj, int last)
{
    uint32_t timeout;
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    if(i2c_p == NULL){
        while(1);
        //return;
    }

    timeout = FLAG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_RXNE) == 0 ){
        timeout--;
        if( timeout == 0 ){
            return -1;
        }
    }

    return (int)(i2c_p->RXDR);
}

int hal_i2c_byte_write(i2c_t *obj, int data)
{
    uint32_t timeout;
    hal_i2c_t *i2c_p = (hal_i2c_t *)(obj->I2c);

    if(i2c_p == NULL){
        while(1);
        //return;
    }

    timeout = FLAG_TIMEOUT;
    while( (i2c_p->ISR & I2C_ISR_TXIS) == 0 ){
        timeout--;
        if( timeout == 0 ){
            return -1;
        }
    }

    i2c_p->TXDR = (uint8_t)data;

    return 1;
}
