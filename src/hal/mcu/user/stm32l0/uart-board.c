/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "uart-board.h"

static uart_tx_call_back_t tx_call_back = NULL;
static uart_rx_call_back_t rx_call_back = NULL;

static Gpio_t uart_tx;
static Gpio_t uart_rx;

static bool uart_busy_flag;
static uint32_t hal_uart_br;

static TimerEvent_t uart_timer;

bool uart_wakeup_flag = false;

void hal_uart_wakeup(void);

static void hal_uart_config(uint32_t baudrate)
{
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

#else
#error "Please define UART port first."
#endif

    /* Choose system clock as UART1 clock */
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_USART1SEL) | (RCC_CCIPR_USART1SEL_0);

    /* Enable the peripheral clock USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* Restore baudrate */
    USART1->BRR = baudrate;
    USART1->CR1 = USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_RE | USART_CR1_UE;

    while((USART1->ISR & USART_ISR_TC) != USART_ISR_TC);

    USART1->ICR |= USART_ICR_TCCF;
    USART1->CR1 |= USART_CR1_TCIE;

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}

void hal_uart_init(uart_config_t *config, uart_tx_call_back_t tx_hanlder,
				   uart_rx_call_back_t rx_handler)
{
	tx_call_back = tx_hanlder;
	rx_call_back = rx_handler;
    TimerInit(&uart_timer, hal_uart_wakeup);

	if( tx_call_back == NULL || rx_call_back == NULL ){
		while(1);
	}

	if( config == NULL ){
		while(1);
	}

    hal_uart_config(SystemCoreClock / config->baud);

    uart_busy_flag = true;
    hal_uart_br = USART1->BRR;
}

void hal_uart_wakeup(void)
{
    TimerStop(&uart_timer);

    __disable_irq();
    GpioSetInterrupt( &uart_rx, NO_IRQ, IRQ_HIGH_PRIORITY, NULL );
    uart_busy_flag = true;
    uart_wakeup_flag = true;
    __enable_irq();

    hal_uart_config(hal_uart_br);
}

bool hal_uart_busy(void)
{
    return uart_busy_flag;
}

void hal_uart_deinit_wakeup(int timeout)
{
    __disable_irq();
    hal_uart_br = USART1->BRR;
    NVIC_DisableIRQ(USART1_IRQn);

    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

    RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;

    GpioInit( &uart_tx, UART_TX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &uart_rx, UART_RX, PIN_INPUT, PIN_OPEN_DRAIN, PIN_PULL_DOWN, 0 );
    GpioSetInterrupt( &uart_rx, IRQ_FALLING_EDGE, IRQ_HIGH_PRIORITY, hal_uart_wakeup );

    if(timeout != 0){
        TimerSetValue(&uart_timer, timeout);
        TimerStart(&uart_timer);
    }

    uart_busy_flag = false;
    __enable_irq();
}

void hal_uart_deinit(void)
{
    __disable_irq();
    hal_uart_br = USART1->BRR;
    NVIC_DisableIRQ(USART1_IRQn);

    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;

    RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;

    GpioInit( &uart_tx, UART_TX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &uart_rx, UART_RX, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    uart_busy_flag = false;
    __enable_irq();
}

uint8_t hal_uart_tx_is_empty(void)
{
    if(USART1->ISR & USART_ISR_TXE){
        return 1;
    }
    return 0;
}

void hal_uart_write_tx_reg(uint8_t val)
{
    USART1->TDR = val;
}

void USART1_IRQHandler(void)
{
    uint8_t USART_Data = 0;
    /** uart tx interrupt */
	if((USART1->ISR & USART_ISR_TC) == USART_ISR_TC)
    {
        if( 0 == tx_call_back() ) {
            /** no packet need to transmit, clear interrupt */
            USART1->ICR |= USART_ICR_TCCF; /* Clear transfer complete flag */
        }
    }
    /* Rx and Tx are connected internally, everything sent
      is received as well */
     /** uart rx interrupt */
    else if((USART1->ISR & USART_ISR_RXNE) == USART_ISR_RXNE)
    {
        USART_Data = (uint8_t)(USART1->RDR); /* Receive data, clear flag */
        rx_call_back(USART_Data);
    }
    else
    {
        USART1->ICR |= (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF);
        //while(1);
    }
}


