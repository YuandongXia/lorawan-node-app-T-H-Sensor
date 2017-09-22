/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "board.h"
#include "usb_istr.h"
#include "usb_pwr.h"
#include "usb_prop.h"
#include "usb-cdc.h"

static const char hex_tab[] = "0123456789ABCDEF";

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

//#define USB_CDC_TX_DEBUG
//#define USB_CDC_TX_OPT

static volatile uint32_t usb_cdc_tx_empty;

static volatile uint8_t usb_cdc_rx_buf[USB_CDC_RX_BUF_LEN];
static volatile uint8_t usb_cdc_tx_buf[USB_CDC_TX_BUF_LEN];

static volatile uint8_t usb_cdc_rx_rd_index, usb_cdc_rx_cnt, usb_cdc_rx_wr_index;
static volatile uint8_t usb_cdc_tx_rd_index, usb_cdc_tx_cnt, usb_cdc_tx_wr_index;

usb_cdc_rx_callback_t usb_cdc_rx_callback;

uint8_t usb_cdc_tmp_buf[USB_CDC_TMP_BUF_LEN];

void usb_cdc_init(usb_cdc_rx_callback_t cb)
{
	usb_cdc_rx_rd_index=0, usb_cdc_rx_cnt=0, usb_cdc_rx_wr_index=0;
    usb_cdc_tx_rd_index=0, usb_cdc_tx_cnt=0, usb_cdc_tx_wr_index=0;

	usb_cdc_rx_callback = cb;

	usb_cdc_tx_empty = 1;

    UsbTxLedOff();
    UsbRxLedOff();
}

int usb_cdc_readable(void)
{
	return usb_cdc_rx_cnt;
}

void usb_cdc_putchar(uint8_t c)
{
	usb_cdc_putbuf(&c, 1);
}

void usb_cdc_putstring(char *str)
{
	int len;
	len = strlen(str);

	if( len > 255 ){
		len = 255;
	}

	usb_cdc_putbuf((uint8_t *)str, len);
}

static int16_t usb_cdc_read_tx_buf_byte(void)
{
	uint8_t data;

	if(usb_cdc_tx_cnt == 0){
		return -1;
	}

	/** data to buffer */
	data = usb_cdc_tx_buf[usb_cdc_tx_rd_index++];
	/** TX write pointer point to head */
	if (usb_cdc_tx_rd_index == USB_CDC_TX_BUF_LEN) {
		usb_cdc_tx_rd_index=0;
	}
	/** remain data in TX buffer plus one */
	--usb_cdc_tx_cnt;

	return data;
}

static void usb_cdc_read_tx_buf_bytes(uint8_t *buf, uint8_t len)
{
	int i;
	for(i=0; i<len; i++){
		buf[i] = (uint8_t)usb_cdc_read_tx_buf_byte();
	}
}

#ifdef USB_CDC_TX_DEBUG
void usb_cdc_putbuf(uint8_t *buf, uint8_t len)
{
    uint16_t idx = 0;
    uint16_t usbBufferSize = VIRTUAL_COM_PORT_DATA_SIZE - 1;

    if( !Virtual_ComPort_IsOpen( ) ){
		return;
	}

    while( usb_cdc_tx_empty != 1 );


    while( len > usbBufferSize )
    {
        if( usb_cdc_tx_empty == 1 )
        {
            usb_cdc_tx_empty = 0;

            /* Turn on RX Led*/
            UsbTxLedOn();

            UserToPMABufferCopy( buf + idx, ENDP1_TXADDR, usbBufferSize );
            len -= usbBufferSize;
            idx += usbBufferSize;

            SetEPTxCount( ENDP1, usbBufferSize );
            SetEPTxValid( ENDP1 );
        }
    }

    if( len != 0 )
    {
        // Wait for previous transmission finalization
        while( usb_cdc_tx_empty != 1 );

        usb_cdc_tx_empty = 0;

        /* Turn on RX Led*/
        UsbTxLedOn();

        UserToPMABufferCopy( buf + idx, ENDP1_TXADDR, len );
        SetEPTxCount( ENDP1, len );
        SetEPTxValid( ENDP1 );
    }
}
#elif defined(USB_CDC_TX_OPT)
void usb_cdc_putbuf(uint8_t *buf, uint8_t len)
{
	int save_length, i=0;

	if( !Virtual_ComPort_IsOpen( ) ){
		return;
	}

	/** wait until TX buffer is not full */
    while(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN);

	HAL_USB_CDC_ENTER_MUTEX();
    /** write user data to usb cdc buffer */
    save_length = len;
    i=0;
    while(save_length){
        if(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN){
            HAL_USB_CDC_EXIT_MUTEX();
            while(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN);
            HAL_USB_CDC_ENTER_MUTEX();
        }

        /** data to buffer */
        usb_cdc_tx_buf[usb_cdc_tx_wr_index++]=buf[i];
        i++;
        /** TX write pointer point to head */
        if (usb_cdc_tx_wr_index == USB_CDC_TX_BUF_LEN) {
            usb_cdc_tx_wr_index=0;
        }
        /** remain data in TX buffer plus one */
        ++usb_cdc_tx_cnt;
        save_length--;
    }
    HAL_USB_CDC_EXIT_MUTEX();
}
#else
void usb_cdc_putbuf(uint8_t *buf, uint8_t len)
{
	int save_length, i=0;
//    uint32_t timeout_cnt;

	uint8_t usb_buf_size = VIRTUAL_COM_PORT_DATA_SIZE - 1;
	uint8_t buf_len;

	if( !Virtual_ComPort_IsOpen( ) ){
		return;
	}

//    timeout_cnt = 0;
	/** wait until TX buffer is not full */
    while(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN){
//        timeout_cnt ++;
//        if(timeout_cnt >= 0x1FFFF){
//            SetEPType(ENDP1, EP_BULK);
//            SetEPTxAddr(ENDP1, ENDP1_TXADDR);
//            SetEPTxStatus(ENDP1, EP_TX_NAK);
//            _ClearEP_CTR_TX(EPindex);
//            EP1_IN_Callback();
//        }
    }

	HAL_USB_CDC_ENTER_MUTEX();
	if(usb_cdc_tx_cnt || (!usb_cdc_tx_empty)){
		/** write user data to usb cdc buffer */
		save_length = len;
		i=0;
		while(save_length){
			if(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN){
				HAL_USB_CDC_EXIT_MUTEX();
				while(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN);
				HAL_USB_CDC_ENTER_MUTEX();
			}

			/** data to buffer */
			usb_cdc_tx_buf[usb_cdc_tx_wr_index++]=buf[i];
			i++;
			/** TX write pointer point to head */
			if (usb_cdc_tx_wr_index == USB_CDC_TX_BUF_LEN) {
				usb_cdc_tx_wr_index=0;
			}
			/** remain data in TX buffer plus one */
			++usb_cdc_tx_cnt;
			save_length--;
		}
        HAL_USB_CDC_EXIT_MUTEX();
	}else{
		/** TX endpoint is not empty  */
		usb_cdc_tx_empty = 0;

        /* Turn on RX Led*/
        UsbTxLedOn();

		if(len < usb_buf_size){
			buf_len = len;
		}else{
			/** save remain data */
			save_length = len-usb_buf_size;
			i = usb_buf_size;
			while(save_length){
				if(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN){
					HAL_USB_CDC_EXIT_MUTEX();
					while(usb_cdc_tx_cnt == USB_CDC_TX_BUF_LEN);
					HAL_USB_CDC_ENTER_MUTEX();
				}
				/** data to buffer */
				usb_cdc_tx_buf[usb_cdc_tx_wr_index++]=buf[i];
				i++;
				/** TX write pointer point to head */
				if (usb_cdc_tx_wr_index == USB_CDC_TX_BUF_LEN) {
					usb_cdc_tx_wr_index=0;
				}

				/** remain data in TX buffer plus one */
				++usb_cdc_tx_cnt;

				save_length--;
			}

			buf_len = usb_buf_size;
		}
        HAL_USB_CDC_EXIT_MUTEX();
		/** write data to endpoint */
		UserToPMABufferCopy( buf, ENDP1_TXADDR, buf_len );
		SetEPTxCount( ENDP1, buf_len );
		SetEPTxValid( ENDP1 );
	}
}
#endif

void usb_cdc_putbuf_hex(uint8_t *buf, uint16_t len)
{
	uint8_t tmp_hex;
	int i, tmp;

	if( len > (USB_CDC_TMP_BUF_LEN/3) ){
		/** doesn't get memory, output data one by one */
		for(i=0; i<len; i++){
			tmp_hex = hex_tab[buf[i]>>4];
			usb_cdc_putchar(tmp_hex);
			tmp_hex = hex_tab[buf[i]&0x0F];
			usb_cdc_putchar(tmp_hex);
			usb_cdc_putchar(' ');
		}
	}else{
        int hexstr_len, index = 0;

        hexstr_len = 3*len;
        for(i=0; i<len; i++){
            tmp = 3*i;
            usb_cdc_tmp_buf[tmp] = hex_tab[buf[i]>>4];
            usb_cdc_tmp_buf[tmp+1] = hex_tab[buf[i]&0x0F];
            usb_cdc_tmp_buf[tmp+2] = ' ';
        }
        while(hexstr_len){
            if(hexstr_len >= 255){
                usb_cdc_putbuf(usb_cdc_tmp_buf+index, 255);
                hexstr_len -= 255;
                index += 255;
            }else{
                usb_cdc_putbuf(usb_cdc_tmp_buf+index, hexstr_len);
                hexstr_len = 0;
            }
        }
	}
}

void usb_cdc_puthex(uint8_t val)
{
	usb_cdc_putbuf_hex(&val, 1);
}

void usb_cdc_write_rx_buf_bytes(uint8_t *buf, uint8_t len)
{
	int i;
	for( i=0; i<len; i++ ){
		if( usb_cdc_rx_cnt == USB_CDC_RX_BUF_LEN ){
			/** RX buffer is full */
			break;
		}
		/** data to buffer */
		usb_cdc_rx_buf[usb_cdc_rx_wr_index++]=buf[i];
		/** TX write pointer point to head */
		if (usb_cdc_rx_wr_index == USB_CDC_RX_BUF_LEN) {
			usb_cdc_rx_wr_index=0;
		}
		/** remain data in TX buffer plus one */
		++usb_cdc_rx_cnt;
	}
}

int16_t usb_cdc_getchar(void)
{
	int16_t ret;

	if( usb_cdc_rx_cnt == 0 ){
		return -1;
	}

	if( usb_cdc_rx_buf == NULL ){
		while(1);
	}

	ret = usb_cdc_rx_buf[usb_cdc_rx_rd_index++];
	--usb_cdc_rx_cnt;
	if (usb_cdc_rx_rd_index == USB_CDC_RX_BUF_LEN) {
		usb_cdc_rx_rd_index=0;
	}

    if(usb_cdc_rx_cnt){
        UsbRxLedOff();
    }

	return ret;
}

#ifdef USB_CDC_TX_DEBUG
void EP1_IN_Callback (void)
{
    usb_cdc_tx_empty = 1;
//    SetEPTxStatus(ENDP1, EP_TX_NAK);
    UsbTxLedOff();
}
#elif defined(USB_CDC_TX_OPT)
void EP1_IN_Callback (void)
{
    uint8_t usb_buf_size = VIRTUAL_COM_PORT_DATA_SIZE - 1;
	uint8_t buf_len;
	uint8_t tmp_buf[VIRTUAL_COM_PORT_DATA_SIZE-1];

	if( usb_cdc_tx_cnt == 0 ){
		usb_cdc_tx_empty = 1;
        UsbTxLedOff();
	}else{
		if( usb_cdc_tx_cnt < usb_buf_size ){
			buf_len = usb_cdc_tx_cnt;
		}else{
			/** save remain data */
			buf_len = usb_buf_size;
		}
		usb_cdc_read_tx_buf_bytes(tmp_buf, buf_len);
		UserToPMABufferCopy( tmp_buf, ENDP1_TXADDR, buf_len );
		SetEPTxCount( ENDP1, buf_len );
		SetEPTxValid( ENDP1 );
    }
}
void SOF_Callback(void)
{
    static uint32_t FrameCount = 0;

    if(bDeviceState == CONFIGURED)
    {
        if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
        {
            /* Reset the frame counter */
            FrameCount = 0;

            if(usb_cdc_tx_empty == 0){
                return;
            }
            do{
                /* Check the data to be sent through IN pipe */
                uint8_t usb_buf_size = VIRTUAL_COM_PORT_DATA_SIZE - 1;
                uint8_t buf_len;
                uint8_t tmp_buf[VIRTUAL_COM_PORT_DATA_SIZE-1];

                if( usb_cdc_tx_cnt == 0 ){
                    usb_cdc_tx_empty = 1;
                    UsbTxLedOff();
                }else{
                    if( usb_cdc_tx_cnt < usb_buf_size ){
                        buf_len = usb_cdc_tx_cnt;
                    }else{
                        /** save remain data */
                        buf_len = usb_buf_size;
                    }
                    usb_cdc_read_tx_buf_bytes(tmp_buf, buf_len);
                    UserToPMABufferCopy( tmp_buf, ENDP1_TXADDR, buf_len );
                    SetEPTxCount( ENDP1, buf_len );
                    SetEPTxValid( ENDP1 );
                }
            }while(0);
        }
    }
}
#else
void EP1_IN_Callback (void)
{
	uint8_t usb_buf_size = VIRTUAL_COM_PORT_DATA_SIZE - 1;
	uint8_t buf_len;
	uint8_t tmp_buf[VIRTUAL_COM_PORT_DATA_SIZE-1];

	if( usb_cdc_tx_cnt == 0 ){
		usb_cdc_tx_empty = 1;
        UsbTxLedOff();
	}else{
		if( usb_cdc_tx_cnt < usb_buf_size ){
			buf_len = usb_cdc_tx_cnt;
		}else{
			/** save remain data */
			buf_len = usb_buf_size;
		}
		usb_cdc_read_tx_buf_bytes(tmp_buf, buf_len);
		UserToPMABufferCopy( tmp_buf, ENDP1_TXADDR, buf_len );
		SetEPTxCount( ENDP1, buf_len );
		SetEPTxValid( ENDP1 );
    }
}
#endif

void EP3_OUT_Callback(void)
{
	uint8_t len;
	uint8_t tmp_buf[VIRTUAL_COM_PORT_DATA_SIZE-1];
	int i;

    UsbRxLedOn();

    len = GetEPRxCount(ENDP3);
	PMAToUserBufferCopy((uint8_t *)tmp_buf, ENDP3_RXADDR, len);

	if( usb_cdc_rx_callback == NULL ){
		usb_cdc_write_rx_buf_bytes(tmp_buf, len);
	}else{
		for( i=0; i<len; i++ ){
			usb_cdc_rx_callback(tmp_buf[i]);
		}

        UsbRxLedOff();
	}

	SetEPRxValid(ENDP3);
}
