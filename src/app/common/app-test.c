/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "app-test.h"
#include "LoRaMac.h"
#include "cmd.h"

#define APP_RSSI_SAMPLES                            (50)
#define APP_FEI_SAMPLES                             (50)
#define FSK_FDEV                                    50e3       // bps
#define FSK_DATARATE                                100e3       // bps
#define FSK_BANDWIDTH                               200000       // Hz
#define FSK_AFC_BANDWIDTH                           250000      // Hz
#define FSK_PREAMBLE_LENGTH                         32          // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false
#define FSK_TX_CRC                                  true

#define APP_CMD_BUF_SIZE                            (40)

typedef struct{
    uint8_t buf[256];
    uint16_t len;
    int16_t rssi;
    int8_t snr;
}app_rx_pkt_t;

typedef struct{
    app_test_sta_t test_sta;
    app_test_rf_config_t test_rf_config;
    uint32_t test_tx_done;
    uint32_t test_rx_done;
    uint32_t test_tx_to;
    uint32_t test_rx_to;
    uint32_t test_rx_err;
    app_rx_pkt_t rx_pkt;
    uint8_t test_rf_busy;
    int16_t buf[128];
    uint32_t rssi_cnt;
    uint32_t rssi_cnt_max;
    int16_t rssi_max, rssi_min;
    int rssi_sum;
    int16_t fei_max, fei_min;
    int fei_sum;
}app_t;

uint8_t app_cmd_buf[APP_CMD_BUF_SIZE+4];

app_t *app = (app_t *)&Channels;

static RadioEvents_t RadioEvents;

#if 0
app_test_sta_t app_test_sta;
app_test_rf_config_t app_test_rf_config;
uint32_t app_test_tx_done;
uint32_t app_test_rx_done;
uint32_t app_test_tx_to;
uint32_t app_test_rx_to;
uint32_t app_test_rx_err;
static RadioEvents_t RadioEvents;
app_rx_pkt_t app_rx_pkt;
uint8_t app_test_rf_busy = 0;
int16_t app_buf[128];

uint32_t app_rssi_cnt;
int16_t app_rssi_max, app_rssi_min;
int app_rssi_sum;

int16_t app_fei_max, app_fei_min;
int app_fei_sum;
#endif

static void OnRadioTxDone( void )
{
    app->test_tx_done = 1;
    app->test_rf_busy = 0;
}

static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    app->test_rx_done = 1;
    memcpy1(app->rx_pkt.buf, payload, size);
    app->rx_pkt.len = size;
    app->rx_pkt.rssi = rssi;
    app->rx_pkt.snr = snr;
}

static void OnRadioTxTimeout( void )
{
    app->test_tx_to = 1;
    app->test_rf_busy = 0;
}

static void OnRadioRxTimeout( void )
{
    app->test_rx_to = 1;
}

static void OnRadioRxError( void )
{
    app->test_rx_err = 1;
}

void app_test_init(void)
{
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    //app = (app_t *)&Channels;
    memset1((uint8_t *)app, 0, sizeof(app_t));

    app->test_rf_config.frf = 868000000;
    app->test_rf_config.bw = 0;          // [0: 125 kHz,
                                        //  1: 250 kHz,
                                        //  2: 500 kHz,
                                        //  3: Reserved]
    app->test_rf_config.sf = 7;
    app->test_rf_config.cr = 1;          // [1: 4/5,
                                        //  2: 4/6,
                                        //  3: 4/7,
                                        //  4: 4/8]
    app->test_rf_config.pow = 14;
    app->test_rf_config.tx_pr_len = 8;
    app->test_rf_config.rx_pr_len = 8;
    app->test_rf_config.crc = true;
    app->test_rf_config.iq = false;
    app->test_rf_config.pnet = false;

    app->test_rf_busy = 0;
    app_test_set_sta(APP_TEST_STA_IDLE, NULL);
}

void app_test_get_rf_config(app_test_rf_config_t *config)
{
    *config = app->test_rf_config;
}

int app_set_rf(app_test_sta_t sta, app_test_rf_config_t *config)
{
    Radio.Standby();
    Radio.Init( &RadioEvents );

    switch(sta){
    case APP_TEST_STA_IDLE:
        //app_set_rx_config(config);
        app_set_config(APP_CONFIG_RX, config);
        Radio.Sleep();
        break;
    case APP_TEST_STA_TXCW:
        if(config->frf < RF_MID_BAND_THRESH){
            Radio.Write( 0x01, 0x88 );
        }else{
            Radio.Write( 0x01, 0x80 );
        }
        Radio.Write( 0x3D, 0xA1 );
        Radio.Write( 0x36, 0x01 );
        Radio.Write( 0x1e, 0x08 );
        //app_set_tx_config(config);
        app_set_config(APP_CONFIG_TXCW, config);
        Radio.Send( NULL, 0 );
        break;
    case APP_TEST_STA_TXCLORA:
        if(config->frf < RF_MID_BAND_THRESH){
            Radio.Write( 0x01, 0x88 );
        }else{
            Radio.Write( 0x01, 0x80 );
        }
        Radio.Write( 0x1E, Radio.Read(0x1E)|0x08 );
        //app_set_tx_config(config);
        app_set_config(APP_CONFIG_TXCW, config);
        Radio.Send( NULL, 0 );
        break;
    case APP_TEST_STA_TXLRPKT:
        app->test_tx_done = 0;
        app->test_rf_busy = 0;
        //app_set_tx_config(config);
        app_set_config(APP_CONFIG_TX, config);
        //Radio.Write( 0x39, 0x34 );
        break;
    case APP_TEST_STA_LWDL:
        app->test_tx_done = 0;
        app->test_rf_busy = 0;
        //app_set_lwdl_config(config);
        app_set_config(APP_CONFIG_LWDL, config);
        break;
    case APP_TEST_STA_RXLRPKT:
        //app_set_rx_config(config);
        app_set_config(APP_CONFIG_RX, config);
        Radio.Rx( 0 );
        break;
    case APP_TEST_STA_RSSI:
        app->rssi_cnt = 0;
        break;
    default:
        return -1;
    }

    return 0;
}

int app_test_set_rf_config(app_test_rf_config_t *config)
{
    int ret;
    ret = app_test_check_rf_config(config);
    if(ret < 0){
        return ret;
    }

    app->test_rf_config = *config;

    return app_set_rf(app->test_sta, config);
}

int app_test_check_rf_config(app_test_rf_config_t *config)
{
    if(!Radio.CheckRfFrequency(config->frf)){
        return -1;
    }

    if( config->bw != 0 && config->bw != 1 && config->bw != 2){
        return -1;
    }

    if( config->sf > 12 || config->sf < 6 ){
        return -1;
    }

    if( config->cr == 0 || config->cr > 4 ){
        return -1;
    }

    if( config->pow > 20 || config->pow < -2 ){
        return -1;
    }

    if( config->tx_pr_len < 6 ){
        return -1;
    }

    if( config->rx_pr_len < 6 ){
        return -1;
    }

    return 0;
}

void app_test_reload_rf_config(void)
{
    app_test_rf_config_t *config;

    app->test_rf_config.frf = 868000000;
    app->test_rf_config.bw = 0;          // [0: 125 kHz,
                                        //  1: 250 kHz,
                                        //  2: 500 kHz,
                                        //  3: Reserved]
    app->test_rf_config.sf = 7;
    app->test_rf_config.cr = 1;          // [1: 4/5,
                                        //  2: 4/6,
                                        //  3: 4/7,
                                        //  4: 4/8]
    app->test_rf_config.pow = 14;
    app->test_rf_config.tx_pr_len = 8;
    app->test_rf_config.rx_pr_len = 8;

    config = &app->test_rf_config;

    app_set_rf(app->test_sta, config);
}

extern void SetPublicNetwork( bool enable );

int app_set_config(app_config_t ctype, app_test_rf_config_t *config)
{
    bool crc = config->crc;
    bool iq = config->iq;
    bool pnet = config->pnet;
    uint32_t timeout = 8000;

    if( ctype == APP_CONFIG_TXCW ){
        timeout = 0;
    }

    Radio.Standby();
    Radio.SetChannel( config->frf );
    switch(ctype){
    case APP_CONFIG_LWDL:
        crc = false;
        iq = true;
        pnet = true;
    case APP_CONFIG_TXCW:
    case APP_CONFIG_TX:
        Radio.SetTxConfig( MODEM_LORA, config->pow, 0, config->bw, config->sf, config->cr,
                      config->tx_pr_len, false, crc, 0, 0, iq, timeout);
        break;
    case APP_CONFIG_RX:
        /*
        Fixed payload:  false
        CRC ON:         true
        IQ Inverted:    false
        RX Continuous:  ture
        */
        Radio.SetRxConfig( MODEM_LORA, config->bw, config->sf, config->cr, 0,
                          config->rx_pr_len, 0x3FF, false, 0, crc, false, 0, iq, true);

        /* Enable max 256 bytes payload receive */
        Radio.Write( REG_LR_PAYLOADMAXLENGTH, 0xFF );
        break;
    }
    SetPublicNetwork(pnet);
    return 0;
}

int app_test_set_sta(app_test_sta_t sta, app_test_rf_config_t *config)
{
    if(config == NULL){
        config = &app->test_rf_config;
    }

    if(app->test_sta == sta){
        return -1;
    }

    if(app_set_rf(sta, config)<0){
        return -1;
    }

    app->test_sta = sta;

    return 0;
}

uint8_t app_test_get_sta(void)
{
    return app->test_sta;
}

int app_test_tx_pkt(uint8_t *buf, uint8_t size)
{
    if( (app->test_sta != APP_TEST_STA_TXLRPKT) && (app->test_sta != APP_TEST_STA_LWDL) ){
        return -1;
    }

    if(app->test_rf_busy){
        return -1;
    }

    Radio.Standby();
    Radio.Send( buf, size );
    app->test_rf_busy = 1;

    return 0;
}

void app_test_dump_reg(void)
{
    int i;
    uint8_t app_test_buf[0x80];

    app_test_buf[0] = 0;
    Radio.ReadBuffer(1, app_test_buf+1, 0x7F);

    for(i=0; i<8; i++){
        /** dump register */
        //app_putbuf_hex(app_test_buf+16*i, 16);
        //printf("\r\n");
    }
}

int app_test_rssi(uint32_t freq, uint32_t cnt)
{
    int i;

    if(app->test_sta != APP_TEST_STA_RSSI){
        return -1;
    }

    if(app->test_rf_busy){
        return -1;
    }

    if(cnt==0){
        return -1;
    }

    app->rssi_cnt = 0;
    app->rssi_sum = 0;
    app->rssi_max = -200;
    app->rssi_min = 0;

    app->test_rf_busy = 1;

    SX1276SetModem( MODEM_LORA );
    SX1276SetChannel( freq );
    SX1276GetPaSelect( freq );
    SX1276SetOpMode( RF_OPMODE_RECEIVER );
    DelayMs(1);
    for(i=0; i<5; i++){
        SX1276ReadRssi( MODEM_LORA );
        DelayMs(2);
    }

    for(i=0; i<cnt; i++){
        app->buf[i] = SX1276ReadRssi( MODEM_LORA );
        app->rssi_sum += app->buf[i];
        if(app->buf[i] > app->rssi_max){
            app->rssi_max = app->buf[i];
        }
        if(app->buf[i] < app->rssi_min){
            app->rssi_min = app->buf[i];
        }
    }

    app->test_rf_busy = 0;
    app->rssi_sum = app->rssi_sum/(int)cnt;
    app_test_set_sta(APP_TEST_STA_IDLE, NULL);

    /** Read FEI here */
    SX1276SetModem( MODEM_FSK );
    SX1276SetChannel( freq );
    SX1276GetPaSelect( freq );
    SX1276Write(REG_RXCONFIG, 0x0E);
    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0, false, true );
    Radio.Rx( 0 );

    app->fei_sum = 0;
    app->fei_max = -32768;
    app->fei_min = 32767;

    //DelayMs(5);
    for(i=0; i<cnt; i++){
        app->buf[i] = SX1276Read(REG_FEIMSB);
        app->buf[i] <<= 8;
        app->buf[i] |= SX1276Read(REG_FEILSB);
        app->fei_sum += app->buf[i];
        if(app->buf[i] > app->fei_max){
            app->fei_max = app->buf[i];
        }
        if(app->buf[i] < app->fei_min){
            app->fei_min = app->buf[i];
        }
        SX1276Read(REG_FEILSB);
        SX1276Read(REG_FEIMSB);
    }
    app->fei_sum = app->fei_sum/(int)cnt;

    app_cmd_buf[0]=CMD_TEST_SUB_RSSI;
    app_cmd_buf[1]=(uint8_t)(app->rssi_sum>>0);
    app_cmd_buf[2]=(uint8_t)(app->rssi_sum>>8);
    app_cmd_buf[3]=(uint8_t)(app->rssi_max>>0);
    app_cmd_buf[4]=(uint8_t)(app->rssi_max>>8);
    app_cmd_buf[5]=(uint8_t)(app->rssi_min>>0);
    app_cmd_buf[6]=(uint8_t)(app->rssi_min>>8);
    app_cmd_buf[7]=(uint8_t)(app->fei_sum>>0);
    app_cmd_buf[8]=(uint8_t)(app->fei_sum>>8);
    app_cmd_buf[9]=(uint8_t)(app->fei_max>>0);
    app_cmd_buf[10]=(uint8_t)(app->fei_max>>8);
    app_cmd_buf[11]=(uint8_t)(app->fei_min>>0);
    app_cmd_buf[12]=(uint8_t)(app->fei_min>>8);

#if 0
    /** Test code, print result */
    sprintf((char *)app_buf, "FEI: AVG:%d,MAX:%d,MIN:%d,OFT:%d\r\n", \
        app_fei_sum, app_fei_max, app_fei_min, ((int32_t)app_fei_max - (int32_t)app_fei_min));
    gm_putstr((char*)app_buf);

    sprintf((char *)app_buf, "RSSI: AVG:%d,MAX:%d,MIN:%d,OFT:%d\r\n", \
        app_rssi_sum, app_rssi_max, app_rssi_min, ((int32_t)app_rssi_max - (int32_t)app_rssi_min));
    gm_putstr((char*)app_buf);
    /** (END) Test code, print result */
#endif

    gm_tx(CMD_TEST, CMD_OK, app_cmd_buf, 13);
    app_test_set_sta(APP_TEST_STA_IDLE, NULL);

    return 0;
}

void app_test_evt(void)
{
    switch(app->test_sta){
    case APP_TEST_STA_IDLE:

        break;
    case APP_TEST_STA_TXCW:

        break;
    case APP_TEST_STA_TXCLORA:

        break;
    case APP_TEST_STA_TXLRPKT:
        if(app->test_tx_done){
            __disable_irq();
            app->test_tx_done = 0;
            __enable_irq();
            gm_ack(CMD_TEST, CMD_OK);
        }
        break;
    case APP_TEST_STA_LWDL:
        if(app->test_tx_done){
            __disable_irq();
            app->test_tx_done = 0;
            __enable_irq();
            gm_ack(CMD_TEST, CMD_OK);
        }
        break;
    case APP_TEST_STA_RXLRPKT:
        if(app->test_rx_done){
            __disable_irq();
            app->test_rx_done = 0;
            __enable_irq();
            app_cmd_buf[0]=CMD_TEST_SUB_RXPKT;
            app_cmd_buf[1]= (uint8_t)(((uint16_t)app->rx_pkt.rssi)>>0);
            app_cmd_buf[2]= (uint8_t)(((uint16_t)app->rx_pkt.rssi)>>8);
            app_cmd_buf[3]= (uint8_t)app->rx_pkt.snr;
            memcpy(app_cmd_buf+4, app->rx_pkt.buf, \
                app->rx_pkt.len>APP_CMD_BUF_SIZE?APP_CMD_BUF_SIZE:app->rx_pkt.len);
            gm_tx(CMD_TEST, CMD_OK, app_cmd_buf, app->rx_pkt.len+4);
        }
        break;
    case APP_TEST_STA_RSSI:

        break;
    default:
        break;
    }
}
