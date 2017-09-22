/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __CFG_H
#define __CFG_H

#include "board.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "app-test.h"
#include "version.h"

/* Cofiguration magic number */
#define CFG_FLAG_PATTERN                    (0x963BEA05 + CFG_DFT_MODE + CFG_VER + CFG_DFT_BAND)

#define CFG_INDEX_ONE_BTYE_MAX              (CFG_UARTCFG+1)
#define CFG_INDEX_TWO_BTYE_MAX
#define CFG_INDEX_FOUR_BTYE_MAX             (CFG_UARTBR+1)

/* configure index should be 28*/
typedef enum{
    /* 1 bytes */
    CFG_ADR = 0,
    CFG_ATECHO,
    CFG_BAND,
    CFG_CLASS,
    CFG_DFU_APP_SYNC,
    CFG_DFU_COUNT,
    CFG_DFU_FORCE,
    CFG_DR,
    CFG_DUTY_CYCLE,
    CFG_EIRP_DWELL_TIME,
    CFG_FTXP,
    CFG_HW,
    CFG_NET,
    CFG_MC_FLAG,
    CFG_MODE,
    CFG_PLFMT,
    CFG_JDC,
    CFG_PORT,
    CFG_POWER,
    CFG_REPT,
    CFG_RETRY,
    CFG_RXWIN2_DR,
    CFG_RXWIN2_SFBW,
    CFG_SCR,
    CFG_WDT,
    CFG_TEST,
    CFG_UARTCFG,

    /* 2bytes */

    /* 4bytes */
    CFG_ATTIMEOUT,
    CFG_AGGR_DC,
    CFG_AUTO_SLEEP,
    CFG_CDR_MIN_MAX,
    CFG_GPS_LONG,
    CFG_GPS_LAT,
    CFG_DELAY_JRX1,
    CFG_DELAY_JRX2,
    CFG_DELAY_RX1,
    CFG_DELAY_RX2,
    CFG_FLAG,
    CFG_RXWIN2_FREQ,
    CFG_SEQ,
    CFG_THRESHOLD,
    CFG_PERIOD,
    CFG_PEROFT,
    CFG_UARTBR,

    /* MISC*/
    CFG_APPEUI,
    CFG_APPKEY,
    CFG_APPSKEY,
    CFG_CHDR,
    CFG_CHFREQ,
    CFG_CHMSK,
    CFG_CUSTOM_DLDR,
    CFG_CUSTOM_DR,
    CFG_DEVADDR,
    CFG_DEVEUI,
    CFG_EEPROM,
    CFG_NWKSKEY,
    CFG_MC_APPSKEY,
    CFG_MC_DEVADDR,
    CFG_MC_NWKSKEY,
    CFG_RFCFG,
    CFG_RXWIN1_FREQ,
    CFG_VER,
}cfg_index_t;

#define CFG_START_ADDRESS                   (NVM_START_ADDR)

#define CFG_OFST_FLAG                       (0)
#define CFG_OFST_FLAG_LEN                   (4)
#define CFG_OFST_FLAG_ALIGN                 (0)

#define CFG_OFST_VER                        (CFG_OFST_FLAG + CFG_OFST_FLAG_LEN + CFG_OFST_FLAG_ALIGN)
#define CFG_OFST_VER_LEN                    (1*3)
#define CFG_OFST_VER_ALIGN                  (0)

#define CFG_OFST_HW                         (CFG_OFST_VER + CFG_OFST_VER_LEN + CFG_OFST_VER_ALIGN)
#define CFG_OFST_HW_LEN                     (1)
#define CFG_OFST_HW_ALIGN                   (0)

#define CFG_OFST_DEVEUI                     (CFG_OFST_HW + CFG_OFST_HW_LEN + CFG_OFST_HW_ALIGN)
#define CFG_OFST_DEVEUI_LEN                 (8)
#define CFG_OFST_DEVEUI_ALIGN               (0)

#define CFG_OFST_APPEUI                     (CFG_OFST_DEVEUI + CFG_OFST_DEVEUI_LEN + CFG_OFST_DEVEUI_ALIGN)
#define CFG_OFST_APPEUI_LEN                 (8)
#define CFG_OFST_APPEUI_ALIGN               (0)

#define CFG_OFST_APPKEY                     (CFG_OFST_APPEUI + CFG_OFST_APPEUI_LEN + CFG_OFST_APPEUI_ALIGN)
#define CFG_OFST_APPKEY_LEN                 (16)
#define CFG_OFST_APPKEY_ALIGN               (0)

#define CFG_OFST_DEVADDR                    (CFG_OFST_APPKEY + CFG_OFST_APPKEY_LEN + CFG_OFST_APPKEY_ALIGN)
#define CFG_OFST_DEVADDR_LEN                (4)
#define CFG_OFST_DEVADDR_ALIGN              (0)

#define CFG_OFST_NWKSKEY                    (CFG_OFST_DEVADDR + CFG_OFST_DEVADDR_LEN + CFG_OFST_DEVADDR_ALIGN)
#define CFG_OFST_NWKSKEY_LEN                (16)
#define CFG_OFST_NWKSKEY_ALIGN              (0)

#define CFG_OFST_APPSKEY                    (CFG_OFST_NWKSKEY + CFG_OFST_NWKSKEY_LEN + CFG_OFST_NWKSKEY_ALIGN)
#define CFG_OFST_APPSKEY_LEN                (16)
#define CFG_OFST_APPSKEY_ALIGN              (0)

/*Must be 4 bytes alignment*/
#define CFG_OFST_CHFREQ                     (CFG_OFST_APPSKEY + CFG_OFST_APPSKEY_LEN + CFG_OFST_APPSKEY_ALIGN)
#define CFG_OFST_CHFREQ_LEN                 (4*LORA_MAX_CHS)
#define CFG_OFST_CHFREQ_ALIGN               (0)

#define CFG_OFST_CHDR                       (CFG_OFST_CHFREQ + CFG_OFST_CHFREQ_LEN + CFG_OFST_CHFREQ_ALIGN)
#define CFG_OFST_CHDR_LEN                   (1*LORA_MAX_CHS)
#define CFG_OFST_CHDR_ALIGN                 (0)

/*Must be 4 bytes alignment*/
#define CFG_OFST_RXWIN2_FREQ                (CFG_OFST_CHDR + CFG_OFST_CHDR_LEN + CFG_OFST_CHDR_ALIGN)
#define CFG_OFST_RXWIN2_FREQ_LEN            (4)
#define CFG_OFST_RXWIN2_FREQ_ALIGN          (0)

#define CFG_OFST_RXWIN2_DR                  (CFG_OFST_RXWIN2_FREQ + CFG_OFST_RXWIN2_FREQ_LEN + CFG_OFST_RXWIN2_FREQ_ALIGN)
#define CFG_OFST_RXWIN2_DR_LEN              (1)
#define CFG_OFST_RXWIN2_DR_ALIGN            (0)

#define CFG_OFST_ADR                        (CFG_OFST_RXWIN2_DR + CFG_OFST_RXWIN2_DR_LEN + CFG_OFST_RXWIN2_DR_ALIGN)
#define CFG_OFST_ADR_LEN                    (1)
#define CFG_OFST_ADR_ALIGN                  (0)

#define CFG_OFST_DR                         (CFG_OFST_ADR + CFG_OFST_ADR_LEN + CFG_OFST_ADR_ALIGN)
#define CFG_OFST_DR_LEN                     (1)
#define CFG_OFST_DR_ALIGN                   (0)

#define CFG_OFST_POWER                      (CFG_OFST_DR + CFG_OFST_DR_LEN + CFG_OFST_DR_ALIGN)
#define CFG_OFST_POWER_LEN                  (1)
#define CFG_OFST_POWER_ALIGN                (0)

#define CFG_OFST_REPT                       (CFG_OFST_POWER + CFG_OFST_POWER_LEN + CFG_OFST_POWER_ALIGN)
#define CFG_OFST_REPT_LEN                   (1)
#define CFG_OFST_REPT_ALIGN                 (0)

#define CFG_OFST_CLASS                      (CFG_OFST_REPT + CFG_OFST_REPT_LEN + CFG_OFST_REPT_ALIGN)
#define CFG_OFST_CLASS_LEN                  (1)
#define CFG_OFST_CLASS_ALIGN                (0)

#define CFG_OFST_NET                        (CFG_OFST_CLASS + CFG_OFST_CLASS_LEN + CFG_OFST_CLASS_ALIGN)
#define CFG_OFST_NET_LEN                    (1)
#define CFG_OFST_NET_ALIGN                  (0)

#define CFG_OFST_MC_FLAG                    (CFG_OFST_NET + CFG_OFST_NET_LEN + CFG_OFST_NET_ALIGN)
#define CFG_OFST_MC_FLAG_LEN                (1)
#define CFG_OFST_MC_FLAG_ALIGN              (0)

#define CFG_OFST_SEQ                        (CFG_OFST_MC_FLAG + CFG_OFST_MC_FLAG_LEN + CFG_OFST_MC_FLAG_ALIGN)
#define CFG_OFST_SEQ_LEN                    (4)
#define CFG_OFST_SEQ_ALIGN                  (0)

#define CFG_OFST_WDT                        (CFG_OFST_SEQ + CFG_OFST_SEQ_LEN + CFG_OFST_SEQ_ALIGN)
#define CFG_OFST_WDT_LEN                    (1)
#define CFG_OFST_WDT_ALIGN                  (0)

#define CFG_OFST_RXWIN2_BW                  (CFG_OFST_WDT + CFG_OFST_WDT_LEN + CFG_OFST_WDT_ALIGN)
#define CFG_OFST_RXWIN2_BW_LEN              (1)
#define CFG_OFST_RXWIN2_BW_ALIGN            (0)

#define CFG_OFST_RXWIN2_FLAG                (CFG_OFST_RXWIN2_BW + CFG_OFST_RXWIN2_BW_LEN + CFG_OFST_RXWIN2_BW_ALIGN)
#define CFG_OFST_RXWIN2_FLAG_LEN            (1)
#define CFG_OFST_RXWIN2_FLAG_ALIGN          (0)

#define CFG_OFST_RXWIN2_SFBW                (CFG_OFST_RXWIN2_FLAG + CFG_OFST_RXWIN2_FLAG_LEN + CFG_OFST_RXWIN2_FLAG_ALIGN)
#define CFG_OFST_RXWIN2_SFBW_LEN            (1)
#define CFG_OFST_RXWIN2_SFBW_ALIGN          (0)

#define CFG_OFST_DELAY_RX1                  (CFG_OFST_RXWIN2_SFBW + CFG_OFST_RXWIN2_SFBW_LEN + CFG_OFST_RXWIN2_SFBW_ALIGN)
#define CFG_OFST_DELAY_RX1_LEN              (4)
#define CFG_OFST_DELAY_RX1_ALIGN            (0)

#define CFG_OFST_DELAY_RX2                  (CFG_OFST_DELAY_RX1 + CFG_OFST_DELAY_RX1_LEN + CFG_OFST_DELAY_RX1_ALIGN)
#define CFG_OFST_DELAY_RX2_LEN              (4)
#define CFG_OFST_DELAY_RX2_ALIGN            (0)

#define CFG_OFST_DELAY_JRX1                 (CFG_OFST_DELAY_RX2 + CFG_OFST_DELAY_RX2_LEN + CFG_OFST_DELAY_RX2_ALIGN)
#define CFG_OFST_DELAY_JRX1_LEN             (4)
#define CFG_OFST_DELAY_JRX1_ALIGN           (0)

#define CFG_OFST_DELAY_JRX2                 (CFG_OFST_DELAY_JRX1 + CFG_OFST_DELAY_JRX1_LEN + CFG_OFST_DELAY_JRX1_ALIGN)
#define CFG_OFST_DELAY_JRX2_LEN             (4)
#define CFG_OFST_DELAY_JRX2_ALIGN           (0)

#define CFG_OFST_DUTY_CYCLE                 (CFG_OFST_DELAY_JRX2 + CFG_OFST_DELAY_JRX2_LEN + CFG_OFST_DELAY_JRX2_ALIGN)
#define CFG_OFST_DUTY_CYCLE_LEN             (1)
#define CFG_OFST_DUTY_CYCLE_ALIGN           (0)

#define CFG_OFST_RETRY                      (CFG_OFST_DUTY_CYCLE + CFG_OFST_DUTY_CYCLE_LEN + CFG_OFST_DUTY_CYCLE_ALIGN)
#define CFG_OFST_RETRY_LEN                  (1)
#define CFG_OFST_RETRY_ALIGN                (0)

#define CFG_OFST_SCR                        (CFG_OFST_RETRY + CFG_OFST_RETRY_LEN + CFG_OFST_RETRY_ALIGN)
#define CFG_OFST_SCR_LEN                    (1)
#define CFG_OFST_SCR_ALIGN                  (0)

#define CFG_OFST_FTXP                       (CFG_OFST_SCR + CFG_OFST_SCR_LEN + CFG_OFST_SCR_ALIGN)
#define CFG_OFST_FTXP_LEN                   (1)
#define CFG_OFST_FTXP_ALIGN                 (0)

#define CFG_OFST_CDR_MIN_MAX                (CFG_OFST_FTXP + CFG_OFST_FTXP_LEN + CFG_OFST_FTXP_ALIGN)
#define CFG_OFST_CDR_MIN_MAX_LEN            (4)
#define CFG_OFST_CDR_MIN_MAX_ALIGN          (0)

#define CFG_OFST_MC_DEVADDR                 (CFG_OFST_CDR_MIN_MAX + CFG_OFST_CDR_MIN_MAX_LEN + CFG_OFST_CDR_MIN_MAX_ALIGN)
#define CFG_OFST_MC_DEVADDR_LEN             (4)
#define CFG_OFST_MC_DEVADDR_ALIGN           (0)

#define CFG_OFST_MC_NWKSKEY                 (CFG_OFST_MC_DEVADDR + CFG_OFST_MC_DEVADDR_LEN + CFG_OFST_MC_DEVADDR_ALIGN)
#define CFG_OFST_MC_NWKSKEY_LEN             (16)
#define CFG_OFST_MC_NWKSKEY_ALIGN           (0)

#define CFG_OFST_MC_APPSKEY                 (CFG_OFST_MC_NWKSKEY + CFG_OFST_MC_NWKSKEY_LEN + CFG_OFST_NWKSKEY_ALIGN)
#define CFG_OFST_MC_APPSKEY_LEN             (16)
#define CFG_OFST_MC_APPSKEY_ALIGN           (0)

#define CFG_OFST_CHMSK                      (CFG_OFST_MC_APPSKEY + CFG_OFST_MC_APPSKEY_LEN + CFG_OFST_MC_APPSKEY_ALIGN)
#define CFG_OFST_CHMSK_LEN                  (4*3)   //96 channels, 16bits each, 6 group
#define CFG_OFST_CHMSK_ALIGN                (0)

#define CFG_OFST_AGGR_DC                    (CFG_OFST_CHMSK + CFG_OFST_CHMSK_LEN + CFG_OFST_CHMSK_ALIGN)
#define CFG_OFST_AGGR_DC_LEN                (4)
#define CFG_OFST_AGGR_DC_ALIGN              (0)

#define CFG_OFST_THRESHHOLD                 (CFG_OFST_AGGR_DC + CFG_OFST_AGGR_DC_LEN + CFG_OFST_AGGR_DC_ALIGN)
#define CFG_OFST_THRESHHOLD_LEN             (4)
#define CFG_OFST_THRESHHOLD_ALIGN           (0)

#define CFG_OFST_UARTBR                     (CFG_OFST_THRESHHOLD+CFG_OFST_THRESHHOLD_LEN+CFG_OFST_THRESHHOLD_ALIGN)
#define CFG_OFST_UARTBR_LEN                 (4)
#define CFG_OFST_UARTBR_ALIGN               (0)

#define CFG_OFST_UARTCFG                    (CFG_OFST_UARTBR + CFG_OFST_UARTBR_LEN + CFG_OFST_UARTBR_ALIGN)
#define CFG_OFST_UARTCFG_LEN                (1)
#define CFG_OFST_UARTCFG_ALIGN              (0)

#define CFG_OFST_ATECHO                     (CFG_OFST_UARTCFG + CFG_OFST_UARTCFG_LEN + CFG_OFST_UARTCFG_ALIGN)
#define CFG_OFST_ATECHO_LEN                 (1)
#define CFG_OFST_ATECHO_ALIGN               (0)

#define CFG_OFST_EIRP_DWELL_TIME            (CFG_OFST_ATECHO + CFG_OFST_ATECHO_LEN + CFG_OFST_ATECHO_ALIGN)
#define CFG_OFST_EIRP_DWELL_TIME_LEN        (1)
#define CFG_OFST_EIRP_DWELL_TIME_ALIGN      (1)

#define CFG_OFST_ATTIMEOUT                  (CFG_OFST_EIRP_DWELL_TIME+CFG_OFST_EIRP_DWELL_TIME_LEN+CFG_OFST_EIRP_DWELL_TIME_ALIGN)
#define CFG_OFST_ATTIMEOUT_LEN              (4)
#define CFG_OFST_ATTIMEOUT_ALIGN            (0)

#define CFG_OFST_AUTO_SLEEP                 (CFG_OFST_ATTIMEOUT+CFG_OFST_ATTIMEOUT_LEN+CFG_OFST_ATTIMEOUT_ALIGN)
#define CFG_OFST_AUTO_SLEEP_LEN             (4)
#define CFG_OFST_AUTO_SLEEP_ALIGN           (0)

#define CFG_OFST_MODE                       (CFG_OFST_AUTO_SLEEP+CFG_OFST_AUTO_SLEEP_LEN+CFG_OFST_AUTO_SLEEP_ALIGN)
#define CFG_OFST_MODE_LEN                   (1)
#define CFG_OFST_MODE_ALIGN                 (0)

#define CFG_OFST_PORT                       (CFG_OFST_MODE + CFG_OFST_MODE_LEN + CFG_OFST_MODE_ALIGN)
#define CFG_OFST_PORT_LEN                   (1)
#define CFG_OFST_PORT_ALIGN                 (0)

#define CFG_OFST_BAND                       (CFG_OFST_PORT + CFG_OFST_PORT_LEN + CFG_OFST_PORT_ALIGN)
#define CFG_OFST_BAND_LEN                   (1)
#define CFG_OFST_BAND_ALIGN                 (0)

#define CFG_OFST_JDC                        (CFG_OFST_BAND + CFG_OFST_BAND_LEN + CFG_OFST_BAND_ALIGN)
#define CFG_OFST_JDC_LEN                    (1)
#define CFG_OFST_JDC_ALIGN                  (0)

#define CFG_OFST_RXWIN1_FREQ                (CFG_OFST_JDC + CFG_OFST_JDC_LEN + CFG_OFST_JDC_ALIGN)
#define CFG_OFST_RXWIN1_FREQ_LEN            (4*LORA_MAX_CHS)
#define CFG_OFST_RXWIN1_FREQ_ALIGN          (0)

#define CFG_OFST_CUSTOM_DR                  (CFG_OFST_RXWIN1_FREQ + CFG_OFST_RXWIN1_FREQ_LEN + CFG_OFST_RXWIN1_FREQ_ALIGN)
#define CFG_OFST_CUSTOM_DR_LEN              (1*16)
#define CFG_OFST_CUSTOM_DR_ALIGN            (0)

#define CFG_OFST_CUSTOM_DLDR                (CFG_OFST_CUSTOM_DR + CFG_OFST_CUSTOM_DR_LEN + CFG_OFST_CUSTOM_DR_ALIGN)
#define CFG_OFST_CUSTOM_DLDR_LEN            (1*16)
#define CFG_OFST_CUSTOM_DLDR_ALIGN          (0)

#define CFG_OFST_PERIOD                     (CFG_OFST_CUSTOM_DLDR + CFG_OFST_CUSTOM_DLDR_LEN + CFG_OFST_CUSTOM_DLDR_ALIGN)
#define CFG_OFST_PERIOD_LEN                 (4)
#define CFG_OFST_PERIOD_ALIGN               (0)

#define CFG_OFST_PEROFT                     (CFG_OFST_PERIOD + CFG_OFST_PERIOD_LEN + CFG_OFST_PERIOD_ALIGN)
#define CFG_OFST_PEROFT_LEN                 (4)
#define CFG_OFST_PEROFT_ALIGN               (0)

#define CFG_OFST_RFCFG                      (CFG_OFST_PEROFT + CFG_OFST_PEROFT_LEN + CFG_OFST_PEROFT_ALIGN)
#define CFG_OFST_RFCFG_LEN                  (12)
#define CFG_OFST_RFCFG_ALIGN                (0)

#define CFG_OFST_TEST                       (CFG_OFST_RFCFG + CFG_OFST_RFCFG_LEN + CFG_OFST_RFCFG_ALIGN)
#define CFG_OFST_TEST_LEN                   (1)
#define CFG_OFST_TEST_ALIGN                 (0)

#define CFG_OFST_PLFMT                      (CFG_OFST_TEST + CFG_OFST_TEST_LEN + CFG_OFST_TEST_ALIGN)
#define CFG_OFST_PLFMT_LEN                  (1)
#define CFG_OFST_PLFMT_ALIGN                (2)

#define CFG_OFST_GPS_LONG                   (CFG_OFST_PLFMT + CFG_OFST_PLFMT_LEN + CFG_OFST_PLFMT_ALIGN)
#define CFG_OFST_GPS_LONG_LEN               (4)
#define CFG_OFST_GPS_LONG_ALIGN             (0)

#define CFG_OFST_GPS_LAT                    (CFG_OFST_GPS_LONG + CFG_OFST_GPS_LONG_LEN + CFG_OFST_GPS_LONG_ALIGN)
#define CFG_OFST_GPS_LAT_LEN                (4)
#define CFG_OFST_GPS_LAT_ALIGN              (0)

#if ( (CFG_OFST_GPS_LAT+CFG_OFST_GPS_LAT_LEN+CFG_OFST_GPS_LAT_ALIGN) >= (1024+512) )
#error "cfg error"
#endif

#define CFG_OFST_EEPROM                     (1024+512)
#define CFG_OFST_EEPROM_LEN                 (1*256)
#define CFG_OFST_EEPROM_ALIGN               (0)

/* IEEE EUI */
#define CFG_IEEE_EUI_PATTERN                (0x89ecF5c4)
#define CFG_IEEE_EUI_FLAG                   (1792)
#define CFG_IEEE_EUI                        (CFG_IEEE_EUI_FLAG+4)
#define CFG_IEEE_EUI_END                    (CFG_IEEE_EUI_FLAG+4+16)

/* HW Version */
#define CFG_HWVER_PATTERN                   (0x89ecF5c4)
#define CFG_HWVER_FLAG                      (CFG_IEEE_EUI_END)
#define CFG_HWVER                           (CFG_HWVER_FLAG+4)
#define CFG_HWVER_END                       (CFG_HWVER_FLAG+4+4)

/* RTC Clock */
#define CFG_OFST_RTC_FLAG                   (2032)   // 4 bytes
#define CFG_OFST_RTC_CLOCK                  (2036)   // 8 bytes

/* DFU address */
#define CFG_OFST_DFU_FORCE                  (2045)
#define CFG_OFST_DFU_APP_SYNC               (2046)
#define CFG_OFST_DFU_COUNT                  (2047)

typedef struct{
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} ver_t;

extern const uint32_t cfg_addr_tab[];

void cfg_init(void);
void cfg_reload(void);

uint32_t cfg_read(cfg_index_t index);
int cfg_write(cfg_index_t index, uint32_t data);

void cfg_read_buf(cfg_index_t index, uint8_t *buf, int len);
int cfg_write_buf(cfg_index_t index, uint8_t *buf, int len);

int cfg_set_ch_freq(int ch, uint32_t freq);
uint32_t cfg_get_ch_freq(int ch);

int cfg_set_ch_dr_range(int ch, uint8_t drrange);
uint8_t cfg_get_ch_dr_range(int ch);

int cfg_set_chmsk(int id, uint16_t chmsk);
uint16_t cfg_get_chmsk(int id);

int cfg_get_rfcfg(app_test_rf_config_t *rfcfg);
int cfg_set_rfcfg(app_test_rf_config_t *rfcfg);

void cfg_set_rxwin1_freq(int ch, uint32_t freq);
uint32_t cfg_get_rxwin1_freq(int ch);

void cfg_set_version(ver_t *ver);
void cfg_get_version(ver_t *ver);

void cfg_update_band(PhyType_t phytype);

#endif
