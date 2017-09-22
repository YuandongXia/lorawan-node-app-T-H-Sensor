/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "board.h"
#include "spi-board.h"

void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
	GpioInit( &obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );

    /* GPIOA 5/6/7 */
	GPIOA->AFR[0] = (GPIOA->AFR[0] &~ (0xFFF00000))\
		| (0 << (5 * 4)) | (0 << (6 * 4)) | (0 << (7 * 4));

	obj->Spi = ( SPI_TypeDef* )SPI1_BASE;

	/* Enable the peripheral clock USART1 */
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	if( nss != NC )
	{
		GpioInit( &obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
		GPIOA->AFR[0] = (GPIOA->AFR[0] &~ (0x000F0000)) | (0 << (4 * 4)) ; /* (2) */
	}
	else
	{
		/** Use soft nss */
		SPI1->CR1 |= ( SPI_CR1_SSM | SPI_CR1_SSI );
	}

	SpiFormat( obj, 8, 0, 0, 0 );

	SpiFrequency( obj, 10000000 );

	SPI1->CR1 |= SPI_CR1_SPE;
}

void SpiDeInit( Spi_t *obj )
{
	SPI1->CR1 &= (uint32_t)~SPI_CR1_SPE;

	/** reset register */

	/** reset io */
	GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
	uint32_t tmpreg;

	/** disable spi first */
	SPI1->CR1 &= ~SPI_CR1_SPE;

    if( ( ( ( bits == 8 ) || ( bits == 16 ) ) == false ) ||
        ( ( ( cpol == 0 ) || ( cpol == 1 ) ) == false ) ||
        ( ( ( cpha == 0 ) || ( cpha == 1 ) ) == false ) )
    {
        // SPI error
        while( 1 );
    }

	/** clear DFF, MSTR, CPOL, CPHA */
	tmpreg = SPI1->CR1;
	tmpreg &= ~(SPI_CR1_CPHA | SPI_CR1_CPOL | SPI_CR1_MSTR | SPI_CR1_DFF);

    tmpreg |= ( slave == 0x01 ) ? 0 : SPI_CR1_MSTR;
    tmpreg |= ( cpol == 0x01 ) ? SPI_CR1_CPOL : 0;
    tmpreg |= ( cpha == 0x01 ) ? SPI_CR1_CPHA : 0;
    tmpreg |= ( bits == 8 ) ? 0 : SPI_CR1_DFF;

	SPI1->CR1 = tmpreg;

	SPI1->I2SCFGR &= (uint16_t)~((uint16_t)SPI_I2SCFGR_I2SMOD);

	/** re-enable spi */
   	SPI1->CR1 |= SPI_CR1_SPE;
}

void SpiFrequency( Spi_t *obj, uint32_t hz )
{
	uint32_t i;

	/** disable spi first */
	SPI1->CR1 &= ~SPI_CR1_SPE;

	for(i=0; i<8; i++){
		if( hz >= ( SystemCoreClock / (1<<(i+1)) ) ){
			break;
		}
	}

	if( i==8 ){
		while(1);
	}

	SPI1->CR1 = ( SPI1->CR1 & ~SPI_CR1_BR ) | ( i<<3 );

	/** re-enable spi */
   	SPI1->CR1 |= SPI_CR1_SPE;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
	if( ( obj == NULL ) || ( obj->Spi ) == NULL )
    {
        return 0;
    }

	/** wait until TX buffer is empty */
	while( (SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE );

	SPI1->DR = outData;

	/** wait until RX buffer is not empty */
	while( (SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE );

	return (SPI1->DR);
}

