#ifndef __GOLLUM_H
#define __GOLLUM_H

#include "uart.h"

//#define GM_DEBUG

#define GM_SERIAL_BAUDRATE                  (115200)
#define GM_BUF_LEN                          (1024)

#define GM_CODE_HEAD                        (0x53)
#define GM_CODE_CMD_ERROR                   (0xFF)

#define GM_OFT_HEAD                         (0)
#define GM_OFT_LEN0                         (1)
#define GM_OFT_LEN1                         (2)
#define GM_OFT_CMD                          (3)

typedef serial_port gm_device_t;
gm_device_t gm_scan(void);
void gm_close(gm_device_t device);
int gm_write(gm_device_t handle, uint8_t cmd, uint8_t *buf, int16_t len);
int gm_read(gm_device_t handle, uint8_t *buf, int timeout);


#endif // __GOLLUM_H
