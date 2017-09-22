/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#include "lw.h"
#include "timer.h"
#include "LoRaMac.h"
#include "board.h"
#include "cfg.h"
#include "led.h"
#include "LoRaMacTest.h"

LoRaMacPrimitives_t LoRaMacPrimitives;
LoRaMacCallback_t LoRaMacCallbacks;

static uint8_t lw_flag = 0;
static lw_sta_t lw_sta;
lw_rx_info_t rx_info;
static lw_mode_t lw_mode;

#ifdef USE_DEBUGGER
LoRaMacEventInfoStatus_t mcps_con_sta, mlme_sta, mcps_ind_sta;
#endif

static void McpsConfirm( McpsConfirm_t *mcpsConfirm )
{
    switch(mcpsConfirm->Status){
    case LORAMAC_EVENT_INFO_STATUS_OK:
        switch( mcpsConfirm->McpsRequest ){
        case MCPS_UNCONFIRMED:
        case MCPS_PROPRIETARY:
            // Check Datarate
            // Check TxPower
            lw_sta = LW_STA_DONE;
            break;
        case MCPS_CONFIRMED:
            lw_sta = LW_STA_ACK;
            break;

        default:
            lw_sta = LW_STA_ERROR;
            break;
        }
        break;
    case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:
        if(mcpsConfirm->McpsRequest == MCPS_CONFIRMED){
            lw_sta = LW_STA_ACK_TIMEOUT;
        }else{
            lw_sta = LW_STA_DONE;
        }
        break;
    case LORAMAC_EVENT_INFO_STATUS_ERROR:
    case LORAMAC_EVENT_INFO_STATUS_RX2_ERROR:
    case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT:
    default:
        lw_sta = LW_STA_ERROR;
        break;
    }

#ifdef USE_DEBUGGER
    mcps_con_sta = mcpsConfirm->Status;
#endif

    //NextTx = true;
    lw_flag |= 1;
}

static void MlmeConfirm( MlmeConfirm_t *mlmeConfirm )
{
    switch( mlmeConfirm->MlmeRequest ){
    case MLME_JOIN:
        // Status is OK, node has joined the network
        switch(mlmeConfirm->Status){
        case LORAMAC_EVENT_INFO_STATUS_OK:
            lw_sta = LW_STA_JOINED;
            break;
        case LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL:
        case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:
            lw_sta = LW_STA_JOIN_FAIL;
            break;
        case LORAMAC_EVENT_INFO_STATUS_ERROR:
        case LORAMAC_EVENT_INFO_STATUS_RX2_ERROR:
        case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT:
            lw_sta = LW_STA_ERROR;
            break;
        }
        break;
    case MLME_LINK_CHECK:
        // Check DemodMargin
        // Check NbGateways
        if(mlmeConfirm->NbGateways != 0){
            rx_info.link_alive = 1;
        }else{
            rx_info.link_alive = 0;
        }
        break;
    default:
        break;
    }

#ifdef USE_DEBUGGER
    mlme_sta = mlmeConfirm->Status;
#endif

    //NextTx = true;
    lw_flag |= 2;
}

static void McpsIndication( McpsIndication_t *mcpsIndication )
{
    switch(mcpsIndication->Status){
    case LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL:
    case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED:
    case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS:
    case LORAMAC_EVENT_INFO_STATUS_ERROR:
    case LORAMAC_EVENT_INFO_STATUS_MIC_FAIL:
        lw_sta = LW_STA_ERROR;
        break;
    case LORAMAC_EVENT_INFO_STATUS_OK:
        rx_info.type = mcpsIndication->McpsIndication;
        rx_info.multicast = mcpsIndication->Multicast;
        rx_info.dr = mcpsIndication->RxDatarate;
        rx_info.fpending = mcpsIndication->FramePending;
        rx_info.win = mcpsIndication->RxSlot;
        rx_info.rssi = mcpsIndication->Rssi;
        rx_info.snr = mcpsIndication->Snr;
        rx_info.size = 0;
        if( mcpsIndication->RxData == true ){
            rx_info.port = mcpsIndication->Port;
            rx_info.size = mcpsIndication->BufferSize;
            rx_info.buf = mcpsIndication->Buffer;
            //memcpy1(rx_info.buf, mcpsIndication->Buffer, mcpsIndication->BufferSize);
        }
        rx_info.maccmd_size = 0;
        if((rx_info.maccmd_size != 0) || (rx_info.size != 0)){
            if(lw_sta == LW_STA_ACK){
                lw_sta = LW_STA_ACK_RX;
            }else{
                lw_sta = LW_STA_RX;
            }
        }
        break;
    }

#ifdef USE_DEBUGGER
    mcps_ind_sta = mcpsIndication->Status;
#endif

    lw_flag |= 4;
}

lw_rx_info_t *lw_init( PhyType_t band, lw_mode_t mode)
{
    MibRequestConfirm_t mibReq;

    LoRaMacPrimitives.MacMcpsConfirm = McpsConfirm;
    LoRaMacPrimitives.MacMcpsIndication = McpsIndication;
    LoRaMacPrimitives.MacMlmeConfirm = MlmeConfirm;
    LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
    LoRaMacInitialization( band, &LoRaMacPrimitives, &LoRaMacCallbacks );

    /** Keys, ID and EUIs **/
    cfg_reload();

    if(mode == LW_MODE_ABP){
        mibReq.Type = MIB_NETWORK_JOINED;
        mibReq.Param.IsNetworkJoined = true;
        LoRaMacMibSetRequestConfirm( &mibReq );

        mibReq.Type = MIB_DEVICE_CLASS;
        mibReq.Param.Class = (DeviceClass_t)cfg_read(CFG_CLASS);
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    lw_mode = mode;
    rx_info.maccmd_size = 0;
    rx_info.size = 0;
    rx_info.link_alive = 0;

    return &rx_info;
}

int lw_join(lw_join_t jtype)
{
    LoRaMacStatus_t status;
    MlmeReq_t mlmeReq;
    MibRequestConfirm_t mibReq;

    if(lw_mode != LW_MODE_OTAA){
        return LORAMAC_STATUS_NOT_OTAA_MODE;
    }

    if( jtype == LW_JOIN_FORCE ){
        mibReq.Type = MIB_NETWORK_JOINED;
        mibReq.Param.IsNetworkJoined = false;
        LoRaMacMibSetRequestConfirm( &mibReq );
    }

    mlmeReq.Type = MLME_JOIN;
    status = LoRaMacMlmeRequest( &mlmeReq );
    if(status == LORAMAC_STATUS_OK){
        led_blink(RF_TX_LED, 800);
        rx_info.maccmd_size = 0;
        rx_info.size = 0;
        rx_info.link_alive = 0;
    }
    return status;
}

int lw_send(Mcps_t type, uint8_t port, uint8_t *buf, int size, int retry, int8_t dr)
{
    LoRaMacStatus_t status;
    McpsReq_t mcpsReq;
    LoRaMacTxInfo_t txInfo;

    // LORAMAC_STATUS_PARAMETER_INVALID
    // LORAMAC_STATUS_LENGTH_ERROR
    // LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR
    // LORAMAC_STATUS_OK
    status = LoRaMacQueryTxPossible( size, &txInfo );

    if( status != LORAMAC_STATUS_OK ){
        return status;
    }else{
        mcpsReq.Type = type;
        switch(type){
        case MCPS_UNCONFIRMED:
            mcpsReq.Req.Unconfirmed.fPort = port;
            mcpsReq.Req.Unconfirmed.fBuffer = buf;
            mcpsReq.Req.Unconfirmed.fBufferSize = size;
            mcpsReq.Req.Unconfirmed.Datarate = dr;
            break;
        case MCPS_CONFIRMED:
            mcpsReq.Req.Confirmed.fPort = port;
            mcpsReq.Req.Confirmed.fBuffer = buf;
            mcpsReq.Req.Confirmed.fBufferSize = size;
            mcpsReq.Req.Confirmed.NbTrials = retry;
            mcpsReq.Req.Confirmed.Datarate = dr;
            break;
        case MCPS_PROPRIETARY:
            mcpsReq.Req.Proprietary.fBuffer = buf;
            mcpsReq.Req.Proprietary.fBufferSize = size;
            mcpsReq.Req.Proprietary.Datarate = dr;
            break;
        }
    }

    status = LoRaMacMcpsRequest( &mcpsReq );
    if(status == LORAMAC_STATUS_OK){
        led_blink(RF_TX_LED, 100);
        rx_info.maccmd_size = 0;
        rx_info.size = 0;
        rx_info.link_alive = 0;
    }
    return status;
}

bool lw_get_evt(lw_sta_t *sta)
{
    __disable_irq();
    if(lw_flag == 0){
        __enable_irq();
        return false;
    }
    lw_flag = 0;
    *sta = lw_sta;
    __enable_irq();
    return true;
}

void lw_evt(void)
{
    uint32_t netid;
    uint32_t devaddr;

    MibRequestConfirm_t mibReq;
    bool lw_done_flag = false;

    if(lw_flag == 0){
        return;
    }

    switch(lw_sta){
    case LW_STA_RX:
        led_blink(RF_RX_LED, 150);
        if( rx_info.win != 2){
            lw_done_flag = true;
        }
        break;
    case LW_STA_ACK_RX:
    case LW_STA_ACK:
        led_blink(RF_RX_LED, 150);
        if( rx_info.win != 2){
            lw_done_flag = true;
        }
        break;
    case LW_STA_ACK_TIMEOUT:
        lw_done_flag = true;
        break;
    case LW_STA_JOINED:
        led_blink(RF_RX_LED, 800);
        mibReq.Type = MIB_NET_ID;
        LoRaMacMibGetRequestConfirm(&mibReq);
        netid = mibReq.Param.NetID;
        mibReq.Type = MIB_DEV_ADDR;
        LoRaMacMibGetRequestConfirm(&mibReq);
        devaddr = mibReq.Param.DevAddr;

        netid = netid;
        devaddr = devaddr;

        lw_done_flag = true;
        break;
    case LW_STA_JOIN_FAIL:
        lw_done_flag = true;
        break;
    case LW_STA_DONE:
        lw_done_flag = true;
        break;
    case LW_STA_ERROR:
        lw_done_flag = true;
        break;
    }
    if(lw_done_flag){

    }
    lw_flag = 0;
}




