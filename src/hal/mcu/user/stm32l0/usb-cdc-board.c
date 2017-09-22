/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "board.h"
#include "usb_pwr.h"
#include "led.h"

static void IntToUnicode( uint32_t value, uint8_t *pbuf, uint8_t len );

extern LINE_CODING linecoding;

bool usb_cdc_rx_led = false;
bool usb_cdc_tx_led = false;

void UsbMcuInit( void )
{
	/** Enable SYSCFG clock */
	RCC->APB2ENR |=  RCC_APB2ENR_SYSCFGEN;

	/** Enable VREFINT and its buffer */
	SYSCFG->CFGR3 |= (SYSCFG_CFGR3_ENREF_HSI48 | SYSCFG_CFGR3_EN_VREFINT);

    /** Enable HSI 48MHz */
	RCC->CRRCR |= RCC_CRRCR_HSI48ON;

	/** Wait until RC48 is ready */
	while( ( RCC->CRRCR & RCC_CRRCR_HSI48RDY ) != RCC_CRRCR_HSI48RDY );

    RCC->APB1RSTR = RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;

#ifdef USE_HSE
    /* PLL clock selected as HSI48 clock */
    RCC->CCIPR |= ~RCC_CCIPR_HSI48SEL;
#else
	/* RC48 clock selected as HSI48 clock */
	RCC->CCIPR |= RCC_CCIPR_HSI48SEL;
#endif

	/* Enable the peripheral clock USB */
    RCC->APB1ENR |= RCC_APB1ENR_USBEN;

	NVIC_SetPriority(USB_IRQn, 2); /* (3) */
    NVIC_EnableIRQ(USB_IRQn); /* (4) */

    USB_Init( );
}

void UsbMcuDeInit( void )
{
    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;

    RCC->APB1ENR &= ~RCC_APB1ENR_USBEN;

    /** Disable HSI 48MHz */
	RCC->CRRCR &= ~RCC_CRRCR_HSI48ON;

    SYSCFG->CFGR3 &= ~(SYSCFG_CFGR3_ENREF_HSI48 | SYSCFG_CFGR3_EN_VREFINT);

    RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_USBRST;
}

void UsbMcuEnterLowPowerMode( void )
{
    /* Set the device state to suspend */
    bDeviceState = SUSPENDED;
}

void UsbMcuLeaveLowPowerMode( void )
{
    DEVICE_INFO *pInfo = &Device_Info;

    /* Set the device state to the correct state */
    if( pInfo->Current_Configuration != 0 )
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
    else
    {
        bDeviceState = ATTACHED;
    }
}

void UsbMcuCableConfig( FunctionalState newState )
{
    if( newState != DISABLE )
    {
		USB->BCDR |= USB_BCDR_DPPU;
    }
    else
    {
		USB->BCDR &= ~USB_BCDR_DPPU;
    }
}

#define ID1                                       ( 0x1FF80050 )
#define ID2                                       ( 0x1FF80054 )
#define ID3                                       ( 0x1FF80064 )

void UsbMcuGetSerialNum( void )
{
    uint32_t deviceSerial0, deviceSerial1, deviceSerial2;

    deviceSerial0 = *( uint32_t* )ID1;
    deviceSerial1 = *( uint32_t* )ID2;
    deviceSerial2 = *( uint32_t* )ID3;

    deviceSerial0 += deviceSerial2;

    if( deviceSerial0 != 0 )
    {
        IntToUnicode( deviceSerial0, &Virtual_Com_Port_StringSerial[2] , 8 );
        IntToUnicode( deviceSerial1, &Virtual_Com_Port_StringSerial[18], 4 );
    }
}

static void IntToUnicode( uint32_t value , uint8_t *pbuf , uint8_t len )
{
    uint8_t idx = 0;

    for( idx = 0; idx < len; idx++ )
    {
        if( ( ( value >> 28 ) ) < 0xA )
        {
            pbuf[2 * idx] = ( value >> 28 ) + '0';
        }
        else
        {
            pbuf[2 * idx] = ( value >> 28 ) + 'A' - 10;
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}

bool UsbMcuIsDeviceConfigured( void )
{
    return Virtual_ComPort_IsOpen( );
}

void USB_IRQHandler(void)
{
    USB_Istr( );
}

void USB_FS_WKUP_IRQHandler(void)
{
//    EXTI_ClearITPendingBit( EXTI_Line18 );
}

void UsbRxLedOn()
{
    usb_cdc_rx_led = true;
	//GpioWrite( &Led5, LED_ON );
}

void UsbRxLedOff()
{
    //GpioWrite( &Led5, LED_OFF );
}

void UsbTxLedOn()
{
    usb_cdc_tx_led = true;
    //GpioWrite( &Led6, LED_ON );
}

void UsbTxLedOff()
{
    //( &Led6, LED_OFF );
}

void usb_cdc_sta(void)
{
    if(usb_cdc_rx_led){
        usb_cdc_rx_led = false;
#ifdef USB_RX_LED
        led_blink(USB_RX_LED, 150);
#endif
    }
    if(usb_cdc_tx_led){
        usb_cdc_tx_led = false;
#ifdef USB_TX_LED
        led_blink(USB_TX_LED, 150);
#endif
    }
}
