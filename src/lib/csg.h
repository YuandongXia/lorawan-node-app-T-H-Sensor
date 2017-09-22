#ifndef __CHINA_STATE_GRID_H
#define __CHINA_STATE_GRID_H

#include <stdint.h>
#include <stdbool.h>
#include "hal-csg.h"

#define CSG_TIMEOUT             (500)

#define CSG_PREAM               (0xFE)
#define CSG_HDR                 (0x68)
#define CSG_END                 (0x16)

#define CSG_POS_HDR0            (0)
#define CSG_POS_ADDR0           (1)
#define CSG_POS_HDR1            (7)
#define CSG_POS_CTL             (8)
#define CSG_POS_LEN             (9)
#define CSG_POS_DATA            (10)

typedef struct{
    bool pream;
    uint8_t addr[6];
    uint8_t ctl;
    uint8_t len;
    uint8_t data[255];
}csg_frame_t;

typedef enum{
    CSG_CMD_ENERGY,
    CMS_CMD_POWER,
    CSG_CMD_VOLTAGE,
    CMS_CMD_CURRENT,

    CSG_CMD_MAX,
}csg_cmd_t;

typedef union{
    uint32_t V;
    uint32_t I;
    uint32_t P;
    uint32_t E;
}csg_para_t;

void csg_init(void);
int csg_get(csg_cmd_t cmd, csg_para_t *para);

int csg_pack(uint8_t *buf, csg_frame_t *frm);
int csg_parse(uint8_t *rbuf, int len, csg_frame_t *frm);

void csg_log(csg_frame_t *frm);
void csg_test(void);

#endif // __CHINA_STATE_GRID_H
