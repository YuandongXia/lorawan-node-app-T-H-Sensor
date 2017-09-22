#ifndef __CMD_H
#define __CMD_H
#include "gollum.h"
#include "cfg.h"

#define CMD_OFT_CMD                     (0)
#define CMD_OFT_TYPE                    (1)
#define CMD_OFT_RET                     (1)

#define CMD_ERROR                       (0xFF)
#define CMD_OK                          (0)
#define CMD_MIN_LEN                     (2)

#define CMD_VER                         (0x01)
//#define CMD_RESET                       (0x02)
#define CMD_ID                          (0x03)
#define CMD_ADR                         (0x04)
#define CMD_DR                          (0x05)
#define CMD_REPT                        (0x06)
#define CMD_POWER                       (0x07)
#define CMD_RXWIN2                      (0x08)
#define CMD_KEY                         (0x09)
//#define CMD_DFU                         (0x0A)
#define CMD_MODE                        (0x0B)
#define CMD_CLASS                       (0x0C)
#define CMD_PORT                        (0x0D)
#define CMD_CH                          (0x0E)      // #
#define CMD_FDEFAULT                    (0x0F)
#define CMD_PERIOD                      (0x10)
#define CMD_TEST                        (0x11)
#define CMD_GPIO                        (0x12)
#define CMD_SLEEP                       (0x13)
#define CMD_THRESHOLD                   (0x14)
#define CMD_RXWIN1                      (0x15)
#define CMD_ULFMT                       (0x16)      // Uplink format
#define CMD_METER_FACTOR                (0x17)
#define CMD_ME1_INIT_PULSE              (0x18)
#define CMD_ME2_INIT_PULSE              (0x19)
#define CMD_GPS                         (0x20)
#define CMD_MC                          (0x21)

#define CMD_ID_SUB_CHK                  (1)
#define CMD_ID_SUB_SET_DEVADDR          (2)
#define CMD_ID_SUB_SET_DEVEUI           (3)
#define CMD_ID_SUB_SET_APPEUI           (4)
#define CMD_ID_LEN_CHK                  (0)
#define CMD_ID_LEN_SET_DEVADDR          (4)
#define CMD_ID_LEN_SET_DEVEUI           (8)
#define CMD_ID_LEN_SET_APPEUI           (8)

#define CMD_FDEFAULT_LEN                (9)

#define CMD_ADR_LEN                     (2)

#define CMD_MEFACTOR_LEN                (3)
#define CMD_ME1_INITPUL_LEN             (5)
#define CMD_ME2_INITPUL_LEN             (5)

#define CMD_DR_SUB_CHECK                (1)
#define CMD_DR_SUB_DR                   (2)
#define CMD_DR_SUB_BAND                 (3)
#define CMD_DR_LEN_CHECK                (0)
#define CMD_DR_LEN_DR                   (1)
#define CMD_DR_LEN_BAND                 (1)

#define CMD_KEY_SUB_NWKSKEY             (1)
#define CMD_KEY_SUB_APPSKEY             (2)
#define CMD_KEY_SUB_APPKEY              (3)
#define CMD_KEY_LEN_NWKSKEY             (16)
#define CMD_KEY_LEN_APPSKEY             (16)
#define CMD_KEY_LEN_APPKEY              (16)

#define CMD_PERIOD_SUB_PER              (1)
#define CMD_PERIOD_SUB_OFT              (2)
#define CMD_PERIOD_LEN_PER              (4)
#define CMD_PERIOD_LEN_OFT              (4)

#define CMD_TEST_SUB_STOP               (1)
#define CMD_TEST_SUB_TXCW               (2)
#define CMD_TEST_SUB_TXCLR              (3)
#define CMD_TEST_SUB_RFCFG              (4)
#define CMD_TEST_SUB_TXPKT              (5)
#define CMD_TEST_SUB_RXPKT              (6)
#define CMD_TEST_SUB_RSSI               (7)
#define CMD_TEST_SUB_LWDL               (8)
#define CMD_TEST_SUB_TEST               (9)
#define CMD_TEST_LEN_STOP               (0)
#define CMD_TEST_LEN_TXCW               (0)
#define CMD_TEST_LEN_TXCLR              (0)
#define CMD_TEST_LEN_RFCFG              (9)
#define CMD_TEST_LEN_TXPKT_MIN          (1)
#define CMD_TEST_LEN_RXPKT              (0)
#define CMD_TEST_LEN_RSSI               (8)
#define CMD_TEST_LEN_LWDL               (0)
#define CMD_TEST_LEN_TEST               (1)

typedef struct{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
}cmd_ver_t;

#define CMD_FLAG_ID_DEVADDR             (0x01)
#define CMD_FLAG_ID_APPEUI              (0x02)
#define CMD_FLAG_ID_DEVEUI              (0x04)
typedef struct{
    uint8_t flag;
    uint8_t devaddr[4];
    uint8_t appeui[8];
    uint8_t deveui[8];
}cmd_id_t;

#define CMD_FLAG_KEY_NWKSKEY            (0x01)
#define CMD_FLAG_KEY_APPSKEY            (0x02)
#define CMD_FLAG_KEY_APPKEY             (0x04)
typedef struct{
    uint8_t flag;
    uint8_t nwkskey[16];
    uint8_t appskey[16];
    uint8_t appkey[16];
}cmd_key_t;

#define CMD_FLAG_DR_DFT_DR              (0x01)
#define CMD_FLAG_DR_ADR_DR              (0x02)
#define CMD_FLAG_DR_SCHM                (0x04)
typedef struct{
    uint8_t flag;
    uint8_t dft_dr;
    uint8_t adr_dr;
    uint8_t schm;
}cmd_dr_t;

typedef struct{
    uint32_t freq;
    uint8_t dr;
}cmd_rxwin2_t;

#define CMD_FLAG_CH0                    (1<<0)
#define CMD_FLAG_CH1                    (1<<1)
#define CMD_FLAG_CH2                    (1<<2)
#define CMD_FLAG_CH3                    (1<<3)
#define CMD_FLAG_CH4                    (1<<4)
#define CMD_FLAG_CH5                    (1<<5)
#define CMD_FLAG_CH6                    (1<<6)
#define CMD_FLAG_CH7                    (1<<7)
#define CMD_FLAG_CH8                    (1<<8)
#define CMD_FLAG_CH9                    (1<<9)
#define CMD_FLAG_CH10                   (1<<10)
#define CMD_FLAG_CH11                   (1<<11)
#define CMD_FLAG_CH12                   (1<<12)
#define CMD_FLAG_CH13                   (1<<13)
#define CMD_FLAG_CH14                   (1<<14)
#define CMD_FLAG_CH15                   (1<<15)

typedef struct{
    int num;
    uint16_t msk[6];
    struct{
        int freq;
        int dr_min;
        int dr_max;
    }tab[LW_CHANNELS_MAX_NUM];
}cmd_ch_t;

typedef struct{
    uint16_t msk[6];
    int freq[LW_CHANNELS_MAX_NUM];
}cmd_rxwin1_t;

#define CMD_FLAG_PERIOD             (0x01)
#define CMD_FLAG_PEROFT             (0x02)
typedef struct{
    uint8_t flag;
    uint32_t period;
    uint32_t peroft;
}cmd_period_t;

typedef struct{
	uint32_t frf;

	uint8_t sf;
	uint32_t bw;
	uint8_t cr;

	int8_t pow;
	uint16_t tx_pr_len;
	uint16_t rx_pr_len;
}cmd_rf_t;

typedef struct{
    bool flag;
    uint8_t devaddr[4];
    uint8_t nwkskey[16];
    uint8_t appskey[16];
    uint32_t dfcnt;
}cmd_mc_t;

int cmd_get_ver(gm_device_t gmsp, cmd_ver_t *ver);

int cmd_get_id(gm_device_t gmsp, cmd_id_t *id);
int cmd_set_id(gm_device_t gmsp, cmd_id_t *id);

int cmd_get_adr(gm_device_t gmsp);
int cmd_set_adr(gm_device_t gmsp, bool sta);

int cmd_get_dr(gm_device_t gmsp, cmd_dr_t *dr);
int cmd_set_dr(gm_device_t gmsp, cmd_dr_t *dr);

int cmd_get_rept(gm_device_t gmsp);
int cmd_set_rept(gm_device_t gmsp, uint8_t rept);

int cmd_get_power(gm_device_t gmsp, int8_t *power);
int cmd_set_power(gm_device_t gmsp, int8_t power);

int cmd_get_mode(gm_device_t gmsp);
int cmd_set_mode(gm_device_t gmsp, uint8_t mode);

int cmd_get_port(gm_device_t gmsp);
int cmd_set_port(gm_device_t gmsp, uint8_t port);

int cmd_get_class(gm_device_t gmsp);
int cmd_set_class(gm_device_t gmsp, uint8_t clss);

int cmd_get_key(gm_device_t gmsp, cmd_key_t *key);
int cmd_set_key(gm_device_t gmsp, cmd_key_t *key);

int cmd_get_ch(gm_device_t gmsp, cmd_ch_t *ch);
int cmd_set_ch(gm_device_t gmsp, cmd_ch_t *ch);

int cmd_get_rxwin1(gm_device_t gmsp, cmd_rxwin1_t *ch);
int cmd_set_rxwin1(gm_device_t gmsp, cmd_rxwin1_t *ch);

int cmd_get_rxwin2(gm_device_t gmsp, cmd_rxwin2_t *rxwin2);
int cmd_set_rxwin2(gm_device_t gmsp, cmd_rxwin2_t *rxwin2);

int cmd_fdefault(gm_device_t gmsp);

int cmd_set_period(gm_device_t gmsp, cmd_period_t *period);
int cmd_get_period(gm_device_t gmsp, cmd_period_t *period);

int cmd_set_test(gm_device_t gmsp, int mode, cmd_rf_t *rfcfg);
int cmd_get_test(gm_device_t gmsp, int *mode, cmd_rf_t *rfcfg);

int cmd_set_threshold(gm_device_t gmsp, int16_t threshold);
int cmd_get_threshold(gm_device_t gmsp, int16_t *threshold);

int cmd_set_ulfmt(gm_device_t gmsp, uint8_t ulfmt);
int cmd_get_ulfmt(gm_device_t gmsp);

int cmd_set_gps(gm_device_t gmsp, float longitude, float latitude);
int cmd_get_gps(gm_device_t gmsp, float *longitude, float *latitude);

int cmd_set_factor(gm_device_t gmsp, int16_t factor);
int cmd_get_factor(gm_device_t gmsp, int16_t *factor);

int cmd_set_me1initpul(gm_device_t gmsp, int32_t me1pul);
int cmd_get_me1initpul(gm_device_t gmsp, int32_t *me1pul);

int cmd_set_me2initpul(gm_device_t gmsp, int32_t me2pul);
int cmd_get_me2initpul(gm_device_t gmsp, int32_t *me2pul);

int cmd_get_mc(gm_device_t gmsp);
int cmd_set_mc(gm_device_t gmsp, cmd_mc_t *mc);

#endif // __CMD_H
