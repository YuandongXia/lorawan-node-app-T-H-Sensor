/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "cfg.h"
#include "cfg-macro.h"
#include "nvm.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "hal-eui.h"
#include "app-test.h"
#include "lw.h"
#include "app.h"

//#define CFG_DEBUG

const uint32_t cfg_addr_tab[] = {
    /** one byte */
    CFG_START_ADDRESS + CFG_OFST_ADR,
    CFG_START_ADDRESS + CFG_OFST_ATECHO,
    CFG_START_ADDRESS + CFG_OFST_BAND,
    CFG_START_ADDRESS + CFG_OFST_CLASS,
    CFG_START_ADDRESS + CFG_OFST_DFU_APP_SYNC,
    CFG_START_ADDRESS + CFG_OFST_DFU_COUNT,
    CFG_START_ADDRESS + CFG_OFST_DFU_FORCE,
    CFG_START_ADDRESS + CFG_OFST_DR,
    CFG_START_ADDRESS + CFG_OFST_DUTY_CYCLE,
    CFG_START_ADDRESS + CFG_OFST_EIRP_DWELL_TIME,
    CFG_START_ADDRESS + CFG_OFST_FTXP,
    CFG_START_ADDRESS + CFG_OFST_HW,
    CFG_START_ADDRESS + CFG_OFST_NET,
    CFG_START_ADDRESS + CFG_OFST_MC_FLAG,
    CFG_START_ADDRESS + CFG_OFST_MODE,
    CFG_START_ADDRESS + CFG_OFST_PLFMT,
    CFG_START_ADDRESS + CFG_OFST_JDC,
    CFG_START_ADDRESS + CFG_OFST_PORT,
    CFG_START_ADDRESS + CFG_OFST_POWER,
    CFG_START_ADDRESS + CFG_OFST_REPT,
    CFG_START_ADDRESS + CFG_OFST_RETRY,
    CFG_START_ADDRESS + CFG_OFST_RXWIN2_DR,
    CFG_START_ADDRESS + CFG_OFST_RXWIN2_SFBW,
    CFG_START_ADDRESS + CFG_OFST_SCR,

    CFG_START_ADDRESS + CFG_OFST_WDT,
	CFG_START_ADDRESS + CFG_OFST_TEST,
    CFG_START_ADDRESS + CFG_OFST_UARTCFG,

    /** two bytes */

    /** four bytes */
    CFG_START_ADDRESS + CFG_OFST_ATTIMEOUT,
    CFG_START_ADDRESS + CFG_OFST_AGGR_DC,
    CFG_START_ADDRESS + CFG_OFST_AUTO_SLEEP,
    CFG_START_ADDRESS + CFG_OFST_CDR_MIN_MAX,
    CFG_START_ADDRESS + CFG_OFST_GPS_LONG,
    CFG_START_ADDRESS + CFG_OFST_GPS_LAT,
    CFG_START_ADDRESS + CFG_OFST_DELAY_JRX1,
    CFG_START_ADDRESS + CFG_OFST_DELAY_JRX2,
    CFG_START_ADDRESS + CFG_OFST_DELAY_RX1,
    CFG_START_ADDRESS + CFG_OFST_DELAY_RX2,
    CFG_START_ADDRESS + CFG_OFST_FLAG,
    CFG_START_ADDRESS + CFG_OFST_RXWIN2_FREQ,
    CFG_START_ADDRESS + CFG_OFST_SEQ,
    CFG_START_ADDRESS + CFG_OFST_THRESHHOLD,
    CFG_START_ADDRESS + CFG_OFST_PERIOD,
    CFG_START_ADDRESS + CFG_OFST_PEROFT,
    CFG_START_ADDRESS + CFG_OFST_UARTBR,

    /** array */
    CFG_START_ADDRESS + CFG_OFST_APPEUI,
    CFG_START_ADDRESS + CFG_OFST_APPKEY,
    CFG_START_ADDRESS + CFG_OFST_APPSKEY,
    CFG_START_ADDRESS + CFG_OFST_CHDR,
    CFG_START_ADDRESS + CFG_OFST_CHFREQ,
    CFG_START_ADDRESS + CFG_OFST_CHMSK,
    CFG_START_ADDRESS + CFG_OFST_CUSTOM_DLDR,
    CFG_START_ADDRESS + CFG_OFST_CUSTOM_DR,
    CFG_START_ADDRESS + CFG_OFST_DEVADDR,
    CFG_START_ADDRESS + CFG_OFST_DEVEUI,
    CFG_START_ADDRESS + CFG_OFST_EEPROM,
    CFG_START_ADDRESS + CFG_OFST_NWKSKEY,
    CFG_START_ADDRESS + CFG_OFST_MC_APPSKEY,
    CFG_START_ADDRESS + CFG_OFST_MC_DEVADDR,
    CFG_START_ADDRESS + CFG_OFST_MC_NWKSKEY,
    CFG_START_ADDRESS + CFG_OFST_RFCFG,
    CFG_START_ADDRESS + CFG_OFST_RXWIN1_FREQ,
    CFG_START_ADDRESS + CFG_OFST_VER,
};

const uint8_t cfg_dft_appeui[LW_EUI_LEN]={
    0x52, 0x69, 0x73, 0x69, 0x6E, 0x67, 0x48, 0x46
};

const uint8_t cfg_dft_key[LW_KEY_LEN]={
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

void cfg_init(void)
{
    uint32_t flag, tmp32;
    uint8_t tmp[16];

    int i;
    ver_t version;
    uint8_t dfu_app_sync;
    float ftmp;

#ifndef RHF1S003
    app_test_rf_config_t rfcfg;
#endif

    dfu_app_sync = cfg_read(CFG_DFU_APP_SYNC);
    if( dfu_app_sync == 0 ){
        cfg_write(CFG_DFU_APP_SYNC, 1);
        cfg_write(CFG_DFU_COUNT, 0);
    }

    /* Only initial configuration when flag is not right and version is not matched */
    flag = cfg_read( CFG_FLAG );
    if( flag == CFG_FLAG_PATTERN ){
        cfg_get_version(&version);
        if( (version.major == VER_MAJOR) && (version.minor == VER_MINOR) && (version.patch == VER_PATCH) ){
            return;
        }else{

        }
    }

    /* Save configuration to NVM */
    /* Hardware version */
    cfg_write(CFG_HW, HW_VER);

    if( nvm_read_dword(CFG_START_ADDRESS + CFG_IEEE_EUI_FLAG) == CFG_IEEE_EUI_PATTERN ){
        uint8_t buf[16];
        nvm_read_byte_buf(CFG_START_ADDRESS + CFG_IEEE_EUI, buf, 16);
        /* AppEui*/
        cfg_write_buf(CFG_APPEUI, buf+0, LW_EUI_LEN);

        /* DevEui*/
        hal_eui64(tmp);
        cfg_write_buf(CFG_DEVEUI, buf+8, LW_EUI_LEN);
    }else{
        /* DevEui*/
        hal_eui64(tmp);
        cfg_write_buf(CFG_DEVEUI, (uint8_t *)tmp, LW_EUI_LEN);

        /* AppEui*/
        cfg_write_buf(CFG_APPEUI, (uint8_t *)cfg_dft_appeui, LW_EUI_LEN);
    }

    /* AppKey*/
    cfg_write_buf(CFG_APPKEY, (uint8_t *)cfg_dft_key, LW_KEY_LEN);

    /* DevAddr */
    srand1( RAND_SEED );
    tmp32 = randr( 0, 0x01FFFFFF );
    tmp[0] = tmp32>>0;
    tmp[1] = tmp32>>8;
    tmp[2] = tmp32>>16;
    tmp[3] = tmp32>>24;
    cfg_write_buf(CFG_DEVADDR, tmp, 4);

    /* NwkSKey */
    cfg_write_buf(CFG_NWKSKEY, (uint8_t *)cfg_dft_key, LW_KEY_LEN);

    /* AppSKey */
    cfg_write_buf(CFG_APPSKEY, (uint8_t *)cfg_dft_key, LW_KEY_LEN);

    /** multicast devaddr, nwkskey, appskey */
    cfg_write(CFG_MC_FLAG, false);
    cfg_write_buf(CFG_MC_DEVADDR, tmp, 4);
    cfg_write_buf(CFG_MC_NWKSKEY, (uint8_t *)cfg_dft_key, LW_KEY_LEN);
    cfg_write_buf(CFG_MC_APPSKEY, (uint8_t *)cfg_dft_key, LW_KEY_LEN);

    cfg_update_band(CFG_DFT_BAND);

    /* Mode */
    cfg_write(CFG_MODE, CFG_DFT_MODE);

    /* Sequencer */
    cfg_write(CFG_SEQ, 1);

    /* Port */
    cfg_write(CFG_PORT, CFG_DFT_PORT);

    /** Set none test mode */
    cfg_write(CFG_TEST, 0);

#ifndef RHF1S003
    rfcfg.frf = 868000000;
    rfcfg.bw = 0;                       // [0: 125 kHz,
                                        //  1: 250 kHz,
                                        //  2: 500 kHz,
                                        //  3: Reserved]
    rfcfg.sf = 7;
    rfcfg.cr = 1;                       // [1: 4/5,
                                        //  2: 4/6,
                                        //  3: 4/7,
                                        //  4: 4/8]
    rfcfg.pow = 14;
    rfcfg.tx_pr_len = 8;
    rfcfg.rx_pr_len = 8;
    rfcfg.crc = true;
    rfcfg.iq = false;
    rfcfg.pnet = false;
    cfg_set_rfcfg(&rfcfg);
#endif

    cfg_write(CFG_PERIOD, CFG_DFT_PERIOD);
    cfg_write(CFG_PEROFT, CFG_DFT_PEROFT);

#ifndef RHF1S003
    cfg_write(CFG_PLFMT, CFG_DFT_PLFMT);
#endif

    /* ATTIMEOUT */
    cfg_write(CFG_ATTIMEOUT, CFG_DFT_ATTIMEOUT);

    /* DutyCycle */
    cfg_write(CFG_AGGR_DC, 0);

    cfg_write(CFG_SCR, false);

    cfg_write(CFG_FTXP, 0);

    /* UART Baud Rate */
    cfg_write(CFG_UARTBR, CFG_DFT_UARTBR);

    /* AUTO SLEEP control */
    cfg_write(CFG_AUTO_SLEEP, 0);

    /* watchdog control */
    cfg_write(CFG_WDT, CFG_DFT_WDT);

    for(i=0; i<256; i++){
        nvm_write_byte(cfg_addr_tab[CFG_EEPROM]+i, 0);
    }

    /* GPS Coordinate */
    ftmp = 0.0;
    cfg_write(CFG_GPS_LONG, *(uint32_t *)&ftmp);
    cfg_write(CFG_GPS_LAT, *(uint32_t *)&ftmp);

    /* Save version and flag bytes last, in case EEPROM writen is interrupted */
    /* Version info */
    version.major = VER_MAJOR;
    version.minor = VER_MINOR;
    version.patch = VER_PATCH;
    cfg_set_version(&version);

    /* Mark flag initialized */
    cfg_write( CFG_FLAG, CFG_FLAG_PATTERN);
}

void cfg_reload(void)
{
    int i;
    uint8_t buf[16];
    MibRequestConfirm_t mibReq;
    ChannelParams_t chpara;

    /* load configuration */
    /* DevEui */
    cfg_read_buf(CFG_DEVEUI, buf, LW_EUI_LEN);
    mibReq.Type = MIB_DEV_EUI;
    mibReq.Param.DevEui = buf;
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* AppEui */
    cfg_read_buf(CFG_APPEUI, buf, LW_EUI_LEN);
    mibReq.Type = MIB_APP_EUI;
    mibReq.Param.AppEui = buf;
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* DevAddr */
    cfg_read_buf(CFG_DEVADDR, buf, 4);
    mibReq.Type = MIB_DEV_ADDR;
    mibReq.Param.DevAddr = (((uint32_t)buf[3])<<24) | (((uint32_t)buf[2])<<16) | \
                            (((uint32_t)buf[1])<<8) | (((uint32_t)buf[0])>>0);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* Application key */
    cfg_read_buf(CFG_APPKEY, buf, LW_KEY_LEN);
    mibReq.Type = MIB_APP_KEY;
    mibReq.Param.AppKey = buf;
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* Network session key */
    cfg_read_buf(CFG_NWKSKEY, buf, LW_KEY_LEN);
    mibReq.Type = MIB_NWK_SKEY;
    mibReq.Param.NwkSKey = buf;
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* Application session key */
    cfg_read_buf(CFG_APPSKEY, buf, LW_KEY_LEN);
    mibReq.Type = MIB_APP_SKEY;
    mibReq.Param.AppSKey = buf;
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* Set channel frequency, DataRate Range and band */
    for(i=0; i<LORA_MAX_CHS; i++){
        chpara.Frequency = cfg_get_ch_freq(i);
        chpara.DrRange.Value = cfg_get_ch_dr_range(i);
        chpara.Band = 0;
        if(chpara.Frequency == 0){
            LoRaMacChannelRemove(i);
        }else{
            LoRaMacChannelAdd( i, chpara );
        }
    }

    /* Set channel mask */
    for(i=0; i<6; i++){
        ChannelsMask[i] = cfg_get_chmsk(i);
        ChannelsMaskDefault[i] = ChannelsMask[i];
    }

    /* RX Window2, SF9 */
    mibReq.Type = MIB_RX2_CHANNEL;
    mibReq.Param.Rx2Channel.Frequency = (uint32_t)cfg_read(CFG_RXWIN2_FREQ);
    mibReq.Param.Rx2Channel.Datarate = (uint8_t)cfg_read(CFG_RXWIN2_DR);
    mibReq.Param.Rx2Channel.DrValue = (uint8_t)cfg_read(CFG_RXWIN2_SFBW);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* ADR configuration */
    mibReq.Type = MIB_ADR;
    mibReq.Param.AdrEnable = (bool)cfg_read(CFG_ADR);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* LoRaWAN current datarate */
    mibReq.Type = MIB_CHANNELS_DEFAULT_DATARATE;
    mibReq.Param.ChannelsDefaultDatarate = (uint8_t)cfg_read(CFG_DR);
    LoRaMacMibSetRequestConfirm( &mibReq );
    mibReq.Type = MIB_CHANNELS_DATARATE;
    mibReq.Param.ChannelsDatarate = (uint8_t)cfg_read(CFG_DR);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* Power */
    mibReq.Type = MIB_POWER_DBM;
    mibReq.Param.PowerDbm = (uint8_t)cfg_read(CFG_POWER);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* LoRaMac packat repetition */
    mibReq.Type = MIB_CHANNELS_NB_REP;
    mibReq.Param.ChannelNbRep = (uint8_t)cfg_read(CFG_REPT);
    LoRaMacMibSetRequestConfirm( &mibReq );

    /* TODO: Class */

    /* TODO Sequencer */

    /* AT TIMEOUT */
    //at_set_timeout((uint32_t)cfg_read(CFG_ATTIMEOUT));

    LORAMAC_RSSI_THRESH = (int16_t)((uint16_t)cfg_read(CFG_THRESHOLD));

    /* Set rxwin1 frequency datarate */
    for(i=0; i<LORA_MAX_CHS; i++){
        mibReq.Type = MIB_CUSTOM_RXWIN1;
        mibReq.Param.RxWin1.Ch = i;
        mibReq.Param.RxWin1.Freq = cfg_get_rxwin1_freq(i);
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    /** Restore delay values */
    mibReq.Type = MIB_RECEIVE_DELAY_1;
    mibReq.Param.ReceiveDelay1 = cfg_read(CFG_DELAY_RX1);
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_RECEIVE_DELAY_2;
    mibReq.Param.ReceiveDelay2 = cfg_read(CFG_DELAY_RX2);
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_JOIN_ACCEPT_DELAY_1;
    mibReq.Param.JoinAcceptDelay1 = cfg_read(CFG_DELAY_JRX1);
    LoRaMacMibSetRequestConfirm( &mibReq );

    mibReq.Type = MIB_JOIN_ACCEPT_DELAY_2;
    mibReq.Param.JoinAcceptDelay2 = cfg_read(CFG_DELAY_JRX2);
    LoRaMacMibSetRequestConfirm( &mibReq );

    LoRaMacTestSetDutyCycleOn(cfg_read(CFG_DUTY_CYCLE));

    AggregatedDCycle = (1<<cfg_read(CFG_AGGR_DC));

    /* Multicast keys */
    cfg_read_buf(CFG_MC_DEVADDR, buf, 4);
    cmd_mc_list.Address = (((uint32_t)buf[3])<<24) | (((uint32_t)buf[2])<<16) | \
                            (((uint32_t)buf[1])<<8) | (((uint32_t)buf[0])>>0);
    cfg_read_buf(CFG_MC_NWKSKEY, buf, LW_KEY_LEN);
    memcpy1(cmd_mc_list.NwkSKey, buf, 16);
    cfg_read_buf(CFG_MC_APPSKEY, buf, LW_KEY_LEN);
    memcpy1(cmd_mc_list.AppSKey, buf, 16);
    cmd_mc_list.Next = NULL;

    if( cfg_read(CFG_MC_FLAG) ){
        mibReq.Param.MulticastList = &cmd_mc_list;
    }else{
        mibReq.Param.MulticastList = NULL;
    }
    LoRaMacMulticastChannelLink(mibReq.Param.MulticastList);

    EirpDwellTime_t eirpdwelltime;
    eirpdwelltime.Value = cfg_read(CFG_EIRP_DWELL_TIME);
    LoRaMacExSetDwellTime(eirpdwelltime.Fields.UplinkDwellTime, eirpdwelltime.Fields.UplinkDwellTime);
    LoRaMacExSetMaxEirp(eirpdwelltime.Fields.MaxEirp);

    DownLinkCounterCheckingRelaxed = cfg_read(CFG_SCR);

    ForceTxPower.Value = cfg_read(CFG_FTXP);

    mibReq.Type = MIB_PUBLIC_NETWORK;
    mibReq.Param.EnablePublicNetwork = (bool)cfg_read(CFG_NET);
    LoRaMacMibSetRequestConfirm( &mibReq );

    LoRaMacJoinDutyCycle = cfg_read(CFG_JDC);

    //uart_auto_sleep = cfg_read(CFG_AUTO_SLEEP);

    /* Transmit duty cycle */
    app_set_period(cfg_read(CFG_PERIOD));

    /* Transmit duty cycle offset */
    app_set_period_oft(cfg_read(CFG_PEROFT));
}

void cfg_update_band(PhyType_t phytype)
{
    int i;

    LoRaMacExInit(phytype);

    cfg_write(CFG_BAND, phytype);

    /* uplink channels */
    for(i=0; i<LORA_MAX_CHS; i++){
        cfg_set_ch_freq(i, Channels[i].Frequency);
        cfg_set_ch_dr_range(i, Channels[i].DrRange.Value);
    }

    /* Channel mask */
    for(i=0; i<6; i++){
        cfg_set_chmsk(i, ChannelsMask[i]);
    }

    /* RXWIN1 flag enable/disable */
    for(i=0; i<LORA_MAX_CHS; i++){
        cfg_set_rxwin1_freq(i, 0);
    }

    /* RXWIN2 */
    cfg_write(CFG_RXWIN2_FREQ, Rx2Channel.Frequency);
    cfg_write(CFG_RXWIN2_DR, Rx2Channel.Datarate);
    cfg_write(CFG_RXWIN2_SFBW, Rx2Channel.DrValue);

    /* Uplink datarate */
    cfg_write(CFG_DR, ChannelsDatarate);

    /* Dutycycle, force to set dutycycle off */
    //cfg_write(CFG_DUTY_CYCLE, LoRaMacTestGetDutyCycleOn());
    cfg_write(CFG_DUTY_CYCLE, false);

    /* Channel Power */
    cfg_write(CFG_POWER, TxPowers[ChannelsTxPower]);

    /* Delay */
    cfg_write(CFG_DELAY_RX1, ReceiveDelay1);
    cfg_write(CFG_DELAY_RX2, ReceiveDelay2);
    cfg_write(CFG_DELAY_JRX1, JoinAcceptDelay1);
    cfg_write(CFG_DELAY_JRX2, JoinAcceptDelay2);

    /* ADR */
    cfg_write(CFG_ADR, AdrCtrlOn);  // 00: false/disable, 01: true/enable

    /* LoRaMac Packet Repetition */
    cfg_write(CFG_REPT, ChannelsNbRep);

    /* LoRaWAN confirmed message retry */
    cfg_write(CFG_RETRY, CFG_DFT_RETRY);

    /* LoRaWAN Class */
    cfg_write(CFG_CLASS, LoRaMacDeviceClass);

    /* NET */
    cfg_write(CFG_NET, PublicNetwork);

    /* THRESHHOLD */
    cfg_write(CFG_THRESHOLD, LORAMAC_RSSI_THRESH);

    /* EIRP_DWELL_TIME */
    cfg_write(CFG_EIRP_DWELL_TIME, EirpDwellTime.Value);

    /* JDC */
    cfg_write(CFG_JDC, LoRaMacJoinDutyCycle);
}

int cfg_set_ch_freq(int ch, uint32_t freq)
{
    nvm_write_dword(cfg_addr_tab[CFG_CHFREQ]+(ch<<2), freq);
    return 0;
}

uint32_t cfg_get_ch_freq(int ch)
{
    return nvm_read_dword(cfg_addr_tab[CFG_CHFREQ]+(ch<<2));
}

#if 0
int cfg_set_custom_dr(int drnum, uint8_t dr)
{
    if((drnum > 15) || (drnum < 0) ){
        return -1;
    }
    nvm_write_byte(cfg_addr_tab[CFG_CUSTOM_DR]+drnum, dr);
    return 0;
}

uint8_t cfg_get_custom_dr(int drnum)
{
    return nvm_read_byte(cfg_addr_tab[CFG_CUSTOM_DR]+drnum);
}

int cfg_set_custom_dldr(int drnum, uint8_t dr)
{
    if((drnum > 15) || (drnum < 0) ){
        return -1;
    }
    nvm_write_byte(cfg_addr_tab[CFG_CUSTOM_DLDR]+drnum, dr);
    return 0;
}

uint8_t cfg_get_custom_dldr(int drnum)
{
    return nvm_read_byte(cfg_addr_tab[CFG_CUSTOM_DLDR]+drnum);
}
#endif

int cfg_set_ch_dr_range(int ch, uint8_t drrange)
{
    nvm_write_byte(cfg_addr_tab[CFG_CHDR]+ch, drrange);
    return 0;
}

uint8_t cfg_get_ch_dr_range(int ch)
{
    return nvm_read_byte(cfg_addr_tab[CFG_CHDR]+ch);
}

void cfg_set_rxwin1_freq(int ch, uint32_t freq)
{
    nvm_write_dword(cfg_addr_tab[CFG_RXWIN1_FREQ]+(ch<<2), freq);
}

uint32_t cfg_get_rxwin1_freq(int ch)
{
    return nvm_read_dword(cfg_addr_tab[CFG_RXWIN1_FREQ]+(ch<<2));
}

int cfg_set_chmsk(int id, uint16_t chmsk)
{
    nvm_write_word(cfg_addr_tab[CFG_CHMSK]+(id<<1), chmsk);
    return 0;
}

uint16_t cfg_get_chmsk(int id)
{
    return nvm_read_word(cfg_addr_tab[CFG_CHMSK]+(id<<1));
}

int cfg_set_rfcfg(app_test_rf_config_t *rfcfg)
{
    nvm_write_dword(cfg_addr_tab[CFG_RFCFG] + 0, rfcfg->frf);
    nvm_write_word(cfg_addr_tab[CFG_RFCFG] + 4, rfcfg->tx_pr_len);
    nvm_write_word(cfg_addr_tab[CFG_RFCFG] + 6, rfcfg->rx_pr_len);
    nvm_write_byte(cfg_addr_tab[CFG_RFCFG] + 8, rfcfg->sf);
    nvm_write_byte(cfg_addr_tab[CFG_RFCFG] + 9, rfcfg->bw);
    nvm_write_byte(cfg_addr_tab[CFG_RFCFG] + 10, rfcfg->pow);
    nvm_write_byte(cfg_addr_tab[CFG_RFCFG] + 11, rfcfg->cr);
    return 0;
}

int cfg_get_rfcfg(app_test_rf_config_t *rfcfg)
{
    rfcfg->frf = nvm_read_dword(cfg_addr_tab[CFG_RFCFG] + 0);
    rfcfg->tx_pr_len = nvm_read_word(cfg_addr_tab[CFG_RFCFG] + 4);
    rfcfg->rx_pr_len = nvm_read_word(cfg_addr_tab[CFG_RFCFG] + 6);
    rfcfg->sf = nvm_read_byte(cfg_addr_tab[CFG_RFCFG] + 8);
    rfcfg->bw = nvm_read_byte(cfg_addr_tab[CFG_RFCFG] + 9);
    rfcfg->pow = nvm_read_byte(cfg_addr_tab[CFG_RFCFG] + 10);
    rfcfg->cr = nvm_read_byte(cfg_addr_tab[CFG_RFCFG] + 11);
    rfcfg->crc = true;
    rfcfg->iq = false;
    rfcfg->pnet = false;
    return 0;
}

void cfg_set_version(ver_t *ver)
{
    nvm_write_byte(cfg_addr_tab[CFG_VER], ver->major);
    nvm_write_byte(cfg_addr_tab[CFG_VER]+1, ver->minor);
    nvm_write_byte(cfg_addr_tab[CFG_VER]+2, ver->patch);
}

void cfg_get_version(ver_t *ver)
{
    ver->major = nvm_read_byte(cfg_addr_tab[CFG_VER]);
    ver->minor = nvm_read_byte(cfg_addr_tab[CFG_VER]+1);
    ver->patch = nvm_read_byte(cfg_addr_tab[CFG_VER]+2);
}

int cfg_write_buf(cfg_index_t index, uint8_t *buf, int len)
{
    nvm_write_byte_buf( cfg_addr_tab[index], buf, len);
    return 0;
}

void cfg_read_buf(cfg_index_t index, uint8_t *buf, int len)
{
    nvm_read_byte_buf( cfg_addr_tab[index], buf, len);
}

int cfg_write(cfg_index_t index, uint32_t data)
{
    if(index < CFG_INDEX_ONE_BTYE_MAX){
        nvm_write_byte(cfg_addr_tab[index], (uint8_t)data);
//    }else if(index < CFG_INDEX_TWO_BTYE_MAX){
//        nvm_write_word(cfg_addr_tab[index], (uint16_t)data);
    }else if(index < CFG_INDEX_FOUR_BTYE_MAX){
        nvm_write_dword(cfg_addr_tab[index], data);
    }
    return 0;
}

uint32_t cfg_read(cfg_index_t index)
{
    uint32_t ret;
    if(index < CFG_INDEX_ONE_BTYE_MAX){
        ret = nvm_read_byte(cfg_addr_tab[index]);
//    }else if(index < CFG_INDEX_TWO_BTYE_MAX){
//        ret = nvm_read_word(cfg_addr_tab[index]);
    }else if(index < CFG_INDEX_FOUR_BTYE_MAX){
        ret = nvm_read_dword(cfg_addr_tab[index]);
    }else{
        return 0;
    }
    return ret;
}
