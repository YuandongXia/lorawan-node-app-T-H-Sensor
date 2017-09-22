/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __APP_TEST_H
#define __APP_TEST_H

#include <stdlib.h>
#include <math.h>
#include "board.h"
#include "utilities.h"
#include "radio.h"
#include "cmd.h"

#define CMD_TEST_TYPE_STOP          CMD_TEST_SUB_STOP
#define CMD_TEST_TYPE_RSSI          CMD_TEST_SUB_RSSI
#define CMD_TEST_TYPE_TXCW          CMD_TEST_SUB_TXCW
#define CMD_TEST_TYPE_TXCLORA       CMD_TEST_SUB_TXCLR
#define CMD_TEST_TYPE_TXLRPKT       CMD_TEST_SUB_TXPKT
#define CMD_TEST_TYPE_RXLRPKT       CMD_TEST_SUB_RXPKT
#define CMD_TEST_TYPE_LWDL          CMD_TEST_SUB_LWDL

typedef struct{
	uint32_t frf;

	uint8_t sf;
	uint8_t bw;
	uint8_t cr;

	int8_t pow;

	uint16_t tx_pr_len;
	uint16_t rx_pr_len;

    bool pnet;
    bool crc;
    bool iq;
}app_test_rf_config_t;

/* Need sync with CMD_TEST_TYPE */
typedef enum{
    APP_TEST_STA_IDLE = CMD_TEST_TYPE_STOP,
    APP_TEST_STA_TXCW = CMD_TEST_TYPE_TXCW,
    APP_TEST_STA_TXCLORA = CMD_TEST_TYPE_TXCLORA,
    APP_TEST_STA_TXLRPKT = CMD_TEST_TYPE_TXLRPKT,
    APP_TEST_STA_RXLRPKT = CMD_TEST_TYPE_RXLRPKT,
    APP_TEST_STA_RSSI = CMD_TEST_TYPE_RSSI,
    APP_TEST_STA_LWDL = CMD_TEST_TYPE_LWDL,
}app_test_sta_t;

typedef enum{
    APP_CONFIG_TX,
    APP_CONFIG_TXCW,
    APP_CONFIG_RX,
    APP_CONFIG_LWDL,
}app_config_t;

void app_test_init(void);

void app_test_reload_rf_config(void);
void app_test_get_rf_config(app_test_rf_config_t *config);
int app_test_set_rf_config(app_test_rf_config_t *config);
int app_test_check_rf_config(app_test_rf_config_t *config);

int app_set_config(app_config_t ctype, app_test_rf_config_t *config);

int app_test_tx_pkt(uint8_t *buf, uint8_t size);
int app_test_lwdl_tx_pkt(uint8_t *buf, uint8_t size);

int app_test_rssi(uint32_t freq, uint32_t cnt);

int app_test_set_sta(app_test_sta_t sta, app_test_rf_config_t *config);
uint8_t app_test_get_sta(void);

void app_test_evt(void);

void app_test_dump_reg(void);

#endif

