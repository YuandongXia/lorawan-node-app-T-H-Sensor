#include "gollum.h"
#include "print.h"
#include "cmd.h"

static uint8_t gm_rx_buf[GM_BUF_LEN];
static uint8_t gm_tx_buf[GM_BUF_LEN];

static uint16_t gm_crc(uint8_t *buf, int len);
static int gm_check_crc(uint8_t *buf, int len);

#ifdef GM_DEBUG
#define gm_debug(x...)          printf(x)
#define gm_debug_hbuf(x, y)       puthbuf(x, y)
#else
#define gm_debug(x...)
#define gm_debug_hbuf(x, y)
#endif // GM_DEBUG

#define gm_log(x...)            printf(x)
#define gm_log_hbuf(x, y)       puthbuf(x, y)

gm_device_t gm_scan(void)
{
    int iDevice=0, ret;
    serial_port sp;
    char ** acPorts = uart_list_ports();
    const char *acPort;
    uint8_t buf[1024];

    while((acPort=acPorts[iDevice++])){
        //printf("%s\n", acPort);
        sp = uart_open(acPort);
        if ((sp != INVALID_SERIAL_PORT) && (sp != CLAIMED_SERIAL_PORT)) {
            uart_flush_input(sp, true);
            uart_set_speed(sp, GM_SERIAL_BAUDRATE);
            // get version
            gm_write(sp, 0x01, NULL, 0);
            ret = gm_read(sp, buf, 300);
            if(ret > 0){
                if( (buf[0] == 0x01) && (buf[1] == 0x00) && (ret == 5) ){
                    gm_log("RisingHF device(%s) found, Version: %d.%d.%d\n", acPort, buf[2], buf[3], buf[4]);
                    uart_list_ports_free(acPorts);
                    return sp;
                }
            }
            uart_close(sp);
        }
    }

    gm_log("Can't find any RisingHF device\n");
    uart_list_ports_free(acPorts);

    return NULL;
}

void gm_close(gm_device_t device)
{
    if(device != NULL){
        uart_close(device);
    }
}

int gm_write(gm_device_t handle, uint8_t cmd, uint8_t *buf, int16_t len)
{
    int index = GM_OFT_HEAD;
    uint16_t crc;
    int res;

    //uart_flush_input(handle, true);

    gm_tx_buf[index++] = GM_CODE_HEAD;
    gm_tx_buf[index++] = (uint8_t)(len+1);
    gm_tx_buf[index++] = (uint8_t)((len+1)>>8);
    gm_tx_buf[index++] = cmd;
    memcpy(gm_tx_buf+index, buf, len);
    index += len;
    crc = gm_crc(gm_tx_buf, index);
    gm_tx_buf[index++] = (uint8_t)(crc>>0);
    gm_tx_buf[index++] = (uint8_t)(crc>>8);

    gm_debug("TX: ");
    gm_debug_hbuf(gm_tx_buf, index);
    gm_debug("\n");
    res = uart_send(handle, gm_tx_buf, index, 2000);
    if(res != 0){
        return -1;
    }

    return index;
}

int gm_read(gm_device_t handle, uint8_t *buf, int timeout)
{
    int res, index;
    uint16_t len;

    index=0;
    res = uart_receive(handle, gm_rx_buf+index, 3, 0, timeout);
    if (res != 0) {
        gm_debug("RX can't get header\n");
        return -1;
    }

    len = gm_rx_buf[GM_OFT_LEN0] | (gm_rx_buf[GM_OFT_LEN1]<<8);
    index += 3;
    res = uart_receive(handle, gm_rx_buf+index, len+2, 0, timeout);
    if(res != 0) {
        gm_debug("Receive error\n");
        return -1;
    }

    // check crc
    if(0 == gm_check_crc(gm_rx_buf, len+5)){
        gm_debug("RX: ");
        gm_debug_hbuf(gm_rx_buf, len+5);
        gm_debug("\n");
        memcpy(buf, gm_rx_buf+GM_OFT_CMD, len);
        return len;
    }
    gm_debug("CRC error\n");
    return -1;
}


static uint16_t gm_crc(uint8_t *buf, int len)
{
    uint16_t crc = 0;
    uint16_t i = 0;

    while (len--){
        crc = crc^(int)(*buf++) << 8;
        for (i=8; i!=0; i--){
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
    }
    return crc;
}

static int gm_check_crc(uint8_t *buf, int len)
{
    uint16_t crc;
    crc = gm_crc(buf, len-2) ^ ((((uint16_t)buf[len-2])<<0) | (((uint16_t)buf[len-1])<<8));

    if(crc == 0){
        return 0;
    }

#ifdef GM_UNCHECK_CRC
    return 0;
#else
    return -1;
#endif
}



