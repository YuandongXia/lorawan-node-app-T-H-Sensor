/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "hal-uartdma.h"
#include "board.h"

//static Gpio_t uart_tx;
//static Gpio_t uart_rx;

#ifndef UART_BL
#define HAL_UARTD_RX_BUF_SIZE               (100)

uint8_t hal_uartd_rx_index;
uint8_t hal_uartd_rx_buf[HAL_UARTD_RX_BUF_SIZE];
#else
#define HAL_UARTD_RX_BUF_SIZE               (2096)

uint32_t hal_uartd_rx_index;
uint8_t hal_uartd_rx_buf[HAL_UARTD_RX_BUF_SIZE];
#endif

static uart_tx_call_back_t tx_call_back = NULL;
static uart_rx_call_back_t rx_call_back = NULL;

#ifndef UART_BL
static Gpio_t uart_tx;
static Gpio_t uart_rx;

static bool uart_busy_flag;
static uint32_t hal_uart_br;

static TimerEvent_t uart_timer;
TimerTime_t uart_tick;
#endif

bool uart_wakeup_flag = false;

bool uart_auto_sleep = false;

bool uart_idle = false;

bool uart_init_flag = false;

static uartd_parity_t uart_parity;

void hal_uartd_wakeup(void);

#if defined (UART_TXPB6_RXPB7) || defined (UART_TXPA9_RXPA10)

#define USART1_EN
#define USART                   USART1
#define USART_CLK_EN()          do { \
                                    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART1SEL) | (RCC_CCIPR_USART1SEL_0); \
                                    RCC->APB2ENR |= RCC_APB2ENR_USART1EN; \
                                } while(0);
#define USART_RX_DMA_CH         DMA1_Channel3
#define USART_IRQ               USART1_IRQn
#define USART_DMA_IRQ           DMA1_Channel2_3_IRQn
#define USART_IRQ_HANDLER       USART1_IRQHandler
#define USART_DMA_IRQ_HANDLER   DMA1_Channel2_3_IRQHandler
#define USART_DMA_ISR_GIF       DMA_ISR_GIF3

#elif defined (UART_TXPA2_RXPA3)

#define USART2_EN
#define USART                   USART2
#define USART_CLK_EN()          do { \
                                    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART2SEL) | (RCC_CCIPR_USART2SEL_0); \
                                    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; \
                                } while(0);
#define USART_RX_DMA_CH         DMA1_Channel5
#define USART_IRQ               USART2_IRQn
#define USART_DMA_IRQ           DMA1_Channel4_5_6_7_IRQn
#define USART_IRQ_HANDLER       USART2_IRQHandler
#define USART_DMA_IRQ_HANDLER   DMA1_Channel4_5_6_7_IRQHandler
#define USART_DMA_ISR_GIF       DMA_ISR_GIF5

#endif

static void hal_uartd_config(uint32_t baudrate)
{
    uint32_t cr1reg;
    uart_init_flag = true;

#ifdef UART_TXPB6_RXPB7
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    /* GPIO configuration for USART1 signals */
    /* (1) Select AF mode (10) on PB6 and PAB7 */
    /* (2) AF0 for USART1 signals */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE6|GPIO_MODER_MODE7))\
                 | (GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1); /* (1) */
    GPIOB->AFR[0] = (GPIOB->AFR[0] &~ (0xFF000000))\
                  | (0 << (6 * 4)) | (0 << (7 * 4)); /* (2) */

    /* PB7 RX pull-up */
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(GPIO_PUPDR_PUPD7)) |  GPIO_PUPDR_PUPD7_0;
#elif defined(UART_TXPA9_RXPA10)
    /* Enable the peripheral clock of GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    /* GPIO configuration for USART1 signals */
    /* (1) Select AF mode (00) on PA9 and PA10 */
    /* (2) AF4 for USART1 signals */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE9|GPIO_MODER_MODE10))\
                 | (GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1); /* (1) */
    GPIOA->AFR[1] = (GPIOA->AFR[1] &~ (0x00000FF0))\
                  | (4 << ((9-8) * 4)) | (4 << ((10-8) * 4)); /* (2) */

    /* PA10 RX pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~(GPIO_PUPDR_PUPD10)) |  GPIO_PUPDR_PUPD10_0;
#elif defined(UART_TXPA2_RXPA3)
    /* Enable the peripheral clock of GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    /* GPIO configuration for USART2 signals */
    /* (1) Select AF mode (00) on PA2 and PA3 */
    /* (2) AF4 for USART2 signals */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE2|GPIO_MODER_MODE3))\
                 | (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1); /* (1) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] &~ (0x00000FF0))\
                  | (4 << (2 * 4)) | (4 << (3 * 4)); /* (2) */

    /* PA10 RX pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~(GPIO_PUPDR_PUPD3)) |  GPIO_PUPDR_PUPD3_0;
#else
#error "Please define UART port first."
#endif

    /* Choose system clock as UART1 clock */
    /* Enable the peripheral clock USART1 */
    USART_CLK_EN();

    /* Restore baudrate */
    USART->CR1 = 0;
    USART->BRR = baudrate;
    switch(uart_parity){
    case NONE:
        cr1reg = 0;
        break;
    case EVEN:
        cr1reg = USART_CR1_M_0 | USART_CR1_PCE;
        break;
    case ODD:
        cr1reg = USART_CR1_M_0 | USART_CR1_PCE | USART_CR1_PS;
        break;
    }
    //USART->CR1 = USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;
    USART->CR1 = cr1reg;
    USART->CR1 |=  USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_IDLEIE;
    USART->CR3 |= USART_CR3_EIE;
    while((USART->ISR & USART_ISR_TC) != USART_ISR_TC);

    USART->ICR = USART_ICR_TCCF;
    USART->CR1 |= USART_CR1_TCIE;

    /*************************** UART1 DMA *********************************/
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    USART_RX_DMA_CH->CCR &= ~DMA_CCR_EN;
    USART->CR3 |= USART_CR3_DMAR;

//    /** DMA Channel2 for USART1_TX */
//    DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C2S) | (3<<(2-1)*4);

#ifdef USART1_EN
    /** DMA Channel3 for USART1_RX */
    DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C3S) | (3<<(3-1)*4);
#elif defined (USART2_EN)
    /** DMA Channel5 for USART2_RX */
    DMA1_CSELR->CSELR = (DMA1_CSELR->CSELR & ~DMA_CSELR_C5S) | (4<<(5-1)*4);
#endif

    USART_RX_DMA_CH->CCR &= ~DMA_CCR_EN;
    USART_RX_DMA_CH->CPAR = (uint32_t)(&(USART->RDR));
    USART_RX_DMA_CH->CMAR = (uint32_t)(hal_uartd_rx_buf);
    USART_RX_DMA_CH->CNDTR = HAL_UARTD_RX_BUF_SIZE;
    USART_RX_DMA_CH->CCR = DMA_CCR_MINC | DMA_CCR_TEIE | DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_CIRC;
    USART_RX_DMA_CH->CCR |= DMA_CCR_EN;

    // DMA1->IFCR;  // Write 1 clear
    // DMA1->ISR;   // Global (TC, HT,

    NVIC_SetPriority(USART_IRQ, 1);
    NVIC_EnableIRQ(USART_IRQ);
    NVIC_SetPriority(USART_DMA_IRQ, 1);
    NVIC_EnableIRQ(USART_DMA_IRQ);
}

void hal_uartd_init(uartd_config_t *config, uart_tx_call_back_t tx_hanlder,
            uart_rx_call_back_t rx_handler)
{
    tx_call_back = tx_hanlder;
    rx_call_back = rx_handler;
    hal_uartd_rx_index = 0;

#ifndef UART_BL
    TimerInit(&uart_timer, hal_uartd_wakeup);
#endif

    if( tx_call_back == NULL || rx_call_back == NULL ){
        while(1);
    }

    if( config == NULL ){
        while(1);
    }
    uart_parity = config->parity;
    hal_uartd_config(SystemCoreClock / config->baud);

#ifndef UART_BL
    uart_busy_flag = true;
    hal_uart_br = USART->BRR;
#endif
}

#ifndef UART_BL
void hal_uartd_wakeup(void)
{
    __disable_irq();
    if(uart_init_flag){
        __enable_irq();
        return;
    }
    uart_init_flag = true;
    __enable_irq();

    TimerStop(&uart_timer);
    __disable_irq();
    GpioSetInterrupt( &uart_rx, NO_IRQ, IRQ_HIGH_PRIORITY, NULL );
    uart_busy_flag = true;
    uart_wakeup_flag = true;
    __enable_irq();
    //memset1(hal_uartd_rx_buf, 0, sizeof(hal_uartd_rx_buf));
    hal_uartd_config(hal_uart_br);
}

void hal_uartd_tx_wakeup(void)
{
    __disable_irq();
    if(uart_init_flag){
        __enable_irq();
        return;
    }
    uart_init_flag = true;
    __enable_irq();

    TimerStop(&uart_timer);
    __disable_irq();
    GpioSetInterrupt( &uart_rx, NO_IRQ, IRQ_HIGH_PRIORITY, NULL );
    uart_busy_flag = true;
    __enable_irq();
    //memset1(hal_uartd_rx_buf, 0, sizeof(hal_uartd_rx_buf));
    hal_uartd_config(hal_uart_br);
}


bool hal_uartd_busy(void)
{
    return uart_busy_flag;
}

void hal_uartd_deinit_wakeup(int timeout)
{
    __disable_irq();
    hal_uart_br = USART->BRR;

    NVIC_DisableIRQ(USART_IRQ);
    NVIC_DisableIRQ(USART_DMA_IRQ);

#ifdef USART1_EN
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

    RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
#elif defined (USART2_EN)
    RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;

    RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
#endif

    RCC->AHBRSTR |= RCC_AHBRSTR_DMA1RST;
    RCC->AHBRSTR &= ~RCC_AHBRSTR_DMA1RST;
    RCC->AHBENR &= ~RCC_AHBENR_DMA1EN;

    GpioInit( &uart_tx, UART_TX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &uart_rx, UART_RX, PIN_INPUT, PIN_OPEN_DRAIN, PIN_PULL_UP, 0 );
    GpioSetInterrupt( &uart_rx, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, hal_uartd_wakeup );

    if(timeout != 0){
        TimerSetValue(&uart_timer, timeout);
        TimerStart(&uart_timer);
    }

    uart_init_flag = false;
    uart_busy_flag = false;
    __enable_irq();
}
#endif

uint8_t hal_uartd_tx_is_empty(void)
{
    if(USART->ISR & USART_ISR_TXE){
        return 1;
    }
    return 0;
}

void hal_uartd_write_tx_reg(uint8_t val)
{
    USART->TDR = val;
}

bool hal_uartd_rx_busy(void)
{
    bool ret;

    __disable_irq();
    if( HAL_UARTD_RX_BUF_SIZE == (USART_RX_DMA_CH->CNDTR + hal_uartd_rx_index) ){
        ret = false;
    }else{
        ret = true;
    }
    __enable_irq();

    return ret;
}

void USART_DMA_IRQ_HANDLER(void)
{
    int tmp;
#if defined (USART1_EN)
    if( DMA1->ISR & DMA_ISR_GIF3 ){
        if( DMA1->ISR & DMA_ISR_TCIF3 ){
            DMA1->IFCR |= DMA_IFCR_CTCIF3;
            tmp = HAL_UARTD_RX_BUF_SIZE/2;
            while( (tmp>0) && (tmp--) ){
                rx_call_back(hal_uartd_rx_buf[hal_uartd_rx_index++]);
            }
            hal_uartd_rx_index = 0;
        }
        if( DMA1->ISR & DMA_ISR_HTIF3 ){
            tmp = HAL_UARTD_RX_BUF_SIZE/2;
            while( (tmp>0) && (tmp--) ){
                rx_call_back(hal_uartd_rx_buf[hal_uartd_rx_index++]);
            }
            DMA1->IFCR |= DMA_IFCR_CHTIF3;
        }
        if( DMA1->ISR & DMA_ISR_TEIF3 ){
            DMA1->IFCR |= DMA_IFCR_CTEIF3;
        }
    }
#elif defined (USART2_EN)
    if( DMA1->ISR & DMA_ISR_GIF5 ){
        if( DMA1->ISR & DMA_ISR_TCIF5 ){
            DMA1->IFCR |= DMA_IFCR_CTCIF5;
            tmp = HAL_UARTD_RX_BUF_SIZE/2;
            while( (tmp>0) && (tmp--) ){
                rx_call_back(hal_uartd_rx_buf[hal_uartd_rx_index++]);
            }
            hal_uartd_rx_index = 0;
        }
        if( DMA1->ISR & DMA_ISR_HTIF5 ){
            tmp = HAL_UARTD_RX_BUF_SIZE/2;
            while( (tmp>0) && (tmp--) ){
                rx_call_back(hal_uartd_rx_buf[hal_uartd_rx_index++]);
            }
            DMA1->IFCR |= DMA_IFCR_CHTIF5;
        }
        if( DMA1->ISR & DMA_ISR_TEIF5 ){
            DMA1->IFCR |= DMA_IFCR_CTEIF5;
        }
    }
#else
#warn "No USART is enabled"
#endif
}

void USART_IRQ_HANDLER(void)
{
    int tmp;

    /** uart tx interrupt */
    if((USART->ISR & USART_ISR_TC) == USART_ISR_TC){
        if( 0 == tx_call_back() ) {
            /** no packet need to transmit, clear interrupt */
            USART->ICR = USART_ICR_TCCF; /* Clear transfer complete flag */
        }
    }else if((USART->ISR & USART_ISR_IDLE) == USART_ISR_IDLE){
        USART->ICR = USART_ICR_IDLECF;
        tmp = HAL_UARTD_RX_BUF_SIZE - hal_uartd_rx_index - USART_RX_DMA_CH->CNDTR;
        USART_RX_DMA_CH->CCR &= ~DMA_CCR_EN;
        USART_RX_DMA_CH->CNDTR = HAL_UARTD_RX_BUF_SIZE;
        USART_RX_DMA_CH->CCR |= DMA_CCR_EN;
        while( (tmp>0) && (tmp--) ){
            rx_call_back(hal_uartd_rx_buf[hal_uartd_rx_index++]);
        }
        hal_uartd_rx_index = 0;
        uart_idle = true;
#ifndef UART_BL
        uart_tick = TimerGetCurrentTime();
#endif
    }else if((USART->ISR & USART_ISR_RXNE) == USART_ISR_RXNE){
//        uint8_t USART_Data = 0;
//        USART_Data = (uint8_t)(USART->RDR); /* Receive data, clear flag */
//        rx_call_back(USART_Data);
    }else{
        USART->ICR = (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF);
    }
}
