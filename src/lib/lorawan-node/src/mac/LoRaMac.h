/*!
 * \file      LoRaMac.h
 *
 * \brief     LoRa MAC layer implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jäckle ( STACKFORCE )
 *
 * \defgroup  LORAMAC LoRa MAC layer implementation
 *            This module specifies the API implementation of the LoRaMAC layer.
 *            This is a placeholder for a detailed description of the LoRaMac
 *            layer and the supported features.
 * \{
 *
 * \example   classA/LoRaMote/main.c
 *            LoRaWAN class A application example for the LoRaMote.
 *
 * \example   classB/LoRaMote/main.c
 *            LoRaWAN class B application example for the LoRaMote.
 *
 * \example   classC/LoRaMote/main.c
 *            LoRaWAN class C application example for the LoRaMote.
 */
#ifndef __LORAMAC_H__
#define __LORAMAC_H__

// Includes board dependent definitions such as channels frequencies
#include "LoRaMac-board.h"

/*!
 * Beacon interval in ms
 */
#define BEACON_INTERVAL                             128000

/*!
 * Class A&B receive delay 1 in ms
 */
#define RECEIVE_DELAY1                              1000

/*!
 * Class A&B receive delay 2 in ms
 */
#define RECEIVE_DELAY2                              2000

/*!
 * Join accept receive delay 1 in ms
 */
#define JOIN_ACCEPT_DELAY1                          5000

/*!
 * Join accept receive delay 2 in ms
 */
#define JOIN_ACCEPT_DELAY2                          6000

/*!
 * Class A&B maximum receive window delay in ms
 */
#define MAX_RX_WINDOW                               4000

/*!
 * Class A&B maximum receive window delay in ms
 */
#define MAX_TX_TIME                                 4000

/*!
 * Maximum allowed gap for the FCNT field
 */
#define MAX_FCNT_GAP                                16384

/*!
 * ADR acknowledgement counter limit
 */
#define ADR_ACK_LIMIT                               64

/*!
 * Number of ADR acknowledgement requests before returning to default datarate
 */
#define ADR_ACK_DELAY                               32

/*!
 * Number of seconds after the start of the second reception window without
 * receiving an acknowledge.
 * AckTimeout = \ref ACK_TIMEOUT + Random( -\ref ACK_TIMEOUT_RND, \ref ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT                                 2000

/*!
 * Random number of seconds after the start of the second reception window without
 * receiving an acknowledge
 * AckTimeout = \ref ACK_TIMEOUT + Random( -\ref ACK_TIMEOUT_RND, \ref ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT_RND                             1000

/*!
 * Check the Mac layer state every MAC_STATE_CHECK_TIMEOUT in ms
 */
#define MAC_STATE_CHECK_TIMEOUT                     1000

/*!
 * Maximum number of times the MAC layer tries to get an acknowledge.
 */
#define MAX_ACK_RETRIES                             8

/*!
 * RSSI free threshold [dBm]
 */
#define RSSI_FREE_TH                                ( int8_t )( -85 )

/*!
 * Frame direction definition for up-link communications
 */
#define UP_LINK                                     0

/*!
 * Frame direction definition for down-link communications
 */
#define DOWN_LINK                                   1

/*!
 * Sets the length of the LoRaMAC footer field.
 * Mainly indicates the MIC field length
 */
#define LORAMAC_MFR_LEN                             4

/*!
 * Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

/*!
 * Maximum MAC commands buffer size (can be larger than 15, consider port 0)
 */
#define LORA_MAC_COMMAND_BUF_SIZE                   128

 /*!
 * LoRaMac internal state
 */
//uint32_t LoRaMacState;

typedef enum{
    EU868,		// 863 ~ 870
    US915,		// 902 ~ 928
    US915HYBRID,
    CN779,		// 779 ~ 787
    EU433,		// 433 ~ 434
    AU915,
    AU915OLD,		// 915 ~ 928
    CN470,		// 470 ~ 510
    AS923,
    KR920,
    IN865,
    CN470PREQUEL,
    STE920,
    PHYTYPE_MAX,
}PhyType_t;

typedef enum{
    FSK = 0,
    SF5 = 5,
    SF6 = 6,
    SF7 = 7,
    SF8 = 8,
    SF9 = 9,
    SF10 = 10,
    SF11 = 11,
    SF12 = 12,
    SF_RFU = 0x0F,
}LoRaSf_t;

typedef enum{
    BW125 = 0,
    BW250 = 1,
    BW500 = 2,
}LoRaBw_t;

#define DR(sf, bw)              ( (uint8_t)( (sf) | ((bw)<<4) ))
#define DR_RFU                  (0xFF)

/*!
 * LoRaWAN devices classes definition
 */
typedef enum eDeviceClass
{
    /*!
     * LoRaWAN device class A
     *
     * LoRaWAN Specification V1.0.1, chapter 3ff
     */
    CLASS_A,
    /*!
     * LoRaWAN device class B
     *
     * LoRaWAN Specification V1.0.1, chapter 8ff
     */
    CLASS_B,
    /*!
     * LoRaWAN device class C
     *
     * LoRaWAN Specification V1.0.1, chapter 17ff
     */
    CLASS_C,
}DeviceClass_t;

/*!
 * LoRaMAC channels parameters definition
 */
typedef union uDrRange
{
    /*!
     * Byte-access to the bits
     */
    int8_t Value;
    /*!
     * Structure to store the minimum and the maximum datarate
     */
    struct sFields
    {
         /*!
         * Minimum data rate
         *
         * EU868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
         *
         * US915 - [DR_0, DR_1, DR_2, DR_3, DR_4]
         */
        uint8_t Min : 4;
        /*!
         * Maximum data rate
         *
         * EU868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
         *
         * US915 - [DR_0, DR_1, DR_2, DR_3, DR_4]
         */
        uint8_t Max : 4;
    }Fields;
}DrRange_t;

/*!
 * LoRaMAC band parameters definition
 */
typedef struct sBand
{
    /*!
     * Duty cycle
     */
    uint16_t DCycle;
    /*!
     * Maximum Tx power
     */
    int8_t TxMaxPower;
    /*!
     * Time stamp of the last Tx frame
     */
    TimerTime_t LastTxDoneTime;
    /*!
     * Holds the time where the device is off
     */
    TimerTime_t TimeOff;
}Band_t;

/*!
 * LoRaMAC channel definition
 */
typedef struct sChannelParams
{
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Data rate definition
     */
    DrRange_t DrRange;
    /*!
     * Band index
     */
    uint8_t Band;
}ChannelParams_t;

/*!
 * LoRaMAC receive window 2 channel parameters
 */
typedef struct sRx2ChannelParams
{
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Data rate
     *
     * EU868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
     *
     * US915 - [DR_8, DR_9, DR_10, DR_11, DR_12, DR_13]
     */
    uint8_t  Datarate;

    uint8_t DrValue;
}Rx2ChannelParams_t;

/*!
 * LoRaMAC multicast channel parameter
 */
typedef struct sMulticastParams
{
    /*!
     * Address
     */
    uint32_t Address;
    /*!
     * Network session key
     */
    uint8_t NwkSKey[16];
    /*!
     * Application session key
     */
    uint8_t AppSKey[16];
    /*!
     * Downlink counter
     */
    uint32_t DownLinkCounter;
    /*!
     * Reference pointer to the next multicast channel parameters in the list
     */
    struct sMulticastParams *Next;
}MulticastParams_t;

/*!
 * LoRaMAC frame types
 *
 * LoRaWAN Specification V1.0.1, chapter 4.2.1, table 1
 */
typedef enum eLoRaMacFrameType
{
    /*!
     * LoRaMAC join request frame
     */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /*!
     * LoRaMAC join accept frame
     */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /*!
     * LoRaMAC unconfirmed up-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /*!
     * LoRaMAC unconfirmed down-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /*!
     * LoRaMAC confirmed up-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /*!
     * LoRaMAC confirmed down-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /*!
     * LoRaMAC RFU frame
     */
    FRAME_TYPE_RFU                   = 0x06,
    /*!
     * LoRaMAC proprietary frame
     */
    FRAME_TYPE_PROPRIETARY           = 0x07,
}LoRaMacFrameType_t;

/*!
 * LoRaMAC mote MAC commands
 *
 * LoRaWAN Specification V1.0.1, chapter 5, table 4
 */
typedef enum eLoRaMacMoteCmd
{
    /*!
     * LinkCheckReq
     */
    MOTE_MAC_LINK_CHECK_REQ          = 0x02,
    /*!
     * LinkADRAns
     */
    MOTE_MAC_LINK_ADR_ANS            = 0x03,
    /*!
     * DutyCycleAns
     */
    MOTE_MAC_DUTY_CYCLE_ANS          = 0x04,
    /*!
     * RXParamSetupAns
     */
    MOTE_MAC_RX_PARAM_SETUP_ANS      = 0x05,
    /*!
     * DevStatusAns
     */
    MOTE_MAC_DEV_STATUS_ANS          = 0x06,
    /*!
     * NewChannelAns
     */
    MOTE_MAC_NEW_CHANNEL_ANS         = 0x07,
    /*!
     * RXTimingSetupAns
     */
    MOTE_MAC_RX_TIMING_SETUP_ANS     = 0x08,
    /*!
     * TxParamSetupAns
     */
    MOTE_MAC_TX_PARAM_SETUP_ANS      = 0x09,
    /*!
     * DlChannelAns
     */
    MOTE_MAC_DL_CHANNEL_ANS          = 0x0A,
}LoRaMacMoteCmd_t;

/*!
 * LoRaMAC server MAC commands
 *
 * LoRaWAN Specification V1.0.1 chapter 5, table 4
 */
typedef enum eLoRaMacSrvCmd
{
    /*!
     * LinkCheckAns
     */
    SRV_MAC_LINK_CHECK_ANS           = 0x02,
    /*!
     * LinkADRReq
     */
    SRV_MAC_LINK_ADR_REQ             = 0x03,
    /*!
     * DutyCycleReq
     */
    SRV_MAC_DUTY_CYCLE_REQ           = 0x04,
    /*!
     * RXParamSetupReq
     */
    SRV_MAC_RX_PARAM_SETUP_REQ       = 0x05,
    /*!
     * DevStatusReq
     */
    SRV_MAC_DEV_STATUS_REQ           = 0x06,
    /*!
     * NewChannelReq
     */
    SRV_MAC_NEW_CHANNEL_REQ          = 0x07,
    /*!
     * RXTimingSetupReq
     */
    SRV_MAC_RX_TIMING_SETUP_REQ      = 0x08,
    /*!
     * TxParamSetupReq
     */
    SRV_MAC_TX_PARAM_SETUP_REQ       = 0x09,
    /*!
     * DlChannelReq
     */
    SRV_MAC_DL_CHANNEL_REQ           = 0x0A,
}LoRaMacSrvCmd_t;

enum {
    SRV_MAC_LEN_LINK_CHECK_ANS          = 3,
    SRV_MAC_LEN_LINK_ADR_REQ            = 5,
    SRV_MAC_LEN_DUTY_CYCLE_REQ          = 2,
    SRV_MAC_LEN_RX_PARAM_SETUP_REQ      = 5,
    SRV_MAC_LEN_DEV_STATUS_REQ          = 1,
    SRV_MAC_LEN_NEW_CHANNEL_REQ         = 6,
    SRV_MAC_LEN_RX_TIMING_SETUP_REQ     = 2,
    SRV_MAC_LEN_TX_PARAM_SETUP_REQ      = 2,
    SRV_MAC_LEN_DL_CHANNEL_REQ          = 5,
};

enum {
    MOTE_MAC_LEN_LINK_CHECK_REQ         = 1,
    MOTE_MAC_LEN_LINK_ADR_ANS           = 2,
    MOTE_MAC_LEN_DUTY_CYCLE_ANS         = 1,
    MOTE_MAC_LEN_RX_PARAM_SETUP_ANS     = 2,
    MOTE_MAC_LEN_DEV_STATUS_ANS         = 3,
    MOTE_MAC_LEN_NEW_CHANNEL_ANS        = 2,
    MOTE_MAC_LEN_RX_TIMING_SETUP_ANS    = 1,
    MOTE_MAC_LEN_TX_PARAM_SETUP_ANS     = 1,
    MOTE_MAC_LEN_DL_CHANNEL_ANS         = 2,
};

/*!
 * LoRaMAC Battery level indicator
 */
typedef enum eLoRaMacBatteryLevel
{
    /*!
     * External power source
     */
    BAT_LEVEL_EXT_SRC                = 0x00,
    /*!
     * Battery level empty
     */
    BAT_LEVEL_EMPTY                  = 0x01,
    /*!
     * Battery level full
     */
    BAT_LEVEL_FULL                   = 0xFE,
    /*!
     * Battery level - no measurement available
     */
    BAT_LEVEL_NO_MEASURE             = 0xFF,
}LoRaMacBatteryLevel_t;

/*!
 * LoRaMAC header field definition (MHDR field)
 *
 * LoRaWAN Specification V1.0.1, chapter 4.2
 */
typedef union uLoRaMacHeader
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sHdrBits
    {
        /*!
         * Major version
         */
        uint8_t Major           : 2;
        /*!
         * RFU
         */
        uint8_t RFU             : 3;
        /*!
         * Message type
         */
        uint8_t MType           : 3;
    }Bits;
}LoRaMacHeader_t;

/*!
 * LoRaMAC frame control field definition (FCtrl)
 *
 * LoRaWAN Specification V1.0.1, chapter 4.3.1
 */
typedef union uLoRaMacFrameCtrl
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sCtrlBits
    {
        /*!
         * Frame options length
         */
        uint8_t FOptsLen        : 4;
        /*!
         * Frame pending bit
         */
        uint8_t FPending        : 1;
        /*!
         * Message acknowledge bit
         */
        uint8_t Ack             : 1;
        /*!
         * ADR acknowledgment request bit
         */
        uint8_t AdrAckReq       : 1;
        /*!
         * ADR control in frame header
         */
        uint8_t Adr             : 1;
    }Bits;
}LoRaMacFrameCtrl_t;

/*!
 * Enumeration containing the status of the operation of a MAC service
 */
typedef enum eLoRaMacEventInfoStatus
{
    /*!
     * Service performed successfully
     */
    LORAMAC_EVENT_INFO_STATUS_OK = 0,
    /*!
     * An error occured during the execution of the service
     */
    LORAMAC_EVENT_INFO_STATUS_ERROR,
    /*!
     * A Tx timeouit occured
     */
    LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT,
    /*!
     * An Rx timeout occured on receive window 2
     */
    LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT,
    /*!
     * An Rx error occured on receive window 2
     */
    LORAMAC_EVENT_INFO_STATUS_RX2_ERROR,
    /*!
     * An error occured in the join procedure
     */
    LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL,
    /*!
     * A frame with an invalid downlink counter was received. The
     * downlink counter of the frame was equal to the local copy
     * of the downlink counter of the node.
     */
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED,
    /*!
     * The node has lost MAX_FCNT_GAP or more frames.
     */
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS,
    /*!
     * An address error occured
     */
    LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL,
    /*!
     * message integrity check failure
     */
    LORAMAC_EVENT_INFO_STATUS_MIC_FAIL,
}LoRaMacEventInfoStatus_t;

/*!
 * LoRaMac tx/rx operation state
 */
typedef union eLoRaMacFlags_t
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sMacFlagBits
    {
        /*!
         * MCPS-Req pending
         */
        uint8_t McpsReq         : 1;
        /*!
         * MCPS-Ind pending
         */
        uint8_t McpsInd         : 1;
        /*!
         * MLME-Req pending
         */
        uint8_t MlmeReq         : 1;
        /*!
         * MAC cycle done
         */
        uint8_t MacDone         : 1;
        /*!
         *
         */
        uint8_t SkipMcpsIndCb   : 1;
    }Bits;
}LoRaMacFlags_t;

/*!
 *
 * \brief   LoRaMAC data services
 *
 * \details The following table list the primitives which are supported by the
 *          specific MAC data service:
 *
 * Name                  | Request | Indication | Response | Confirm
 * --------------------- | :-----: | :--------: | :------: | :-----:
 * \ref MCPS_UNCONFIRMED | YES     | YES        | NO       | YES
 * \ref MCPS_CONFIRMED   | YES     | YES        | NO       | YES
 * \ref MCPS_MULTICAST   | NO      | YES        | NO       | NO
 * \ref MCPS_PROPRIETARY | YES     | YES        | NO       | YES
 *
 * The following table provides links to the function implementations of the
 * related MCPS primitives:
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MCPS-Request     | \ref LoRaMacMlmeRequest
 * MCPS-Confirm     | MacMcpsConfirm in \ref LoRaMacPrimitives_t
 * MCPS-Indication  | MacMcpsIndication in \ref LoRaMacPrimitives_t
 */
typedef enum eMcps
{
    /*!
     * Unconfirmed LoRaMAC frame
     */
    MCPS_UNCONFIRMED,
    /*!
     * Confirmed LoRaMAC frame
     */
    MCPS_CONFIRMED,
    /*!
     * Proprietary frame
     */
    MCPS_PROPRIETARY,
    /*!
     * Multicast LoRaMAC frame
     */
    MCPS_MULTICAST,
}Mcps_t;

/*!
 * LoRaMAC MCPS-Request for an unconfirmed frame
 */
typedef struct sMcpsReqUnconfirmed
{
    /*!
     * Frame port field. Must be set if the payload is not empty. Use the
     * application specific frame port values: [1...223]
     *
     * LoRaWAN Specification V1.0.1, chapter 4.3.2
     */
    uint8_t fPort;
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
}McpsReqUnconfirmed_t;

/*!
 * LoRaMAC MCPS-Request for a confirmed frame
 */
typedef struct sMcpsReqConfirmed
{
    /*!
     * Frame port field. Must be set if the payload is not empty. Use the
     * application specific frame port values: [1...223]
     *
     * LoRaWAN Specification V1.0.1, chapter 4.3.2
     */
    uint8_t fPort;
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
    /*!
     * Number of trials to transmit the frame, if the LoRaMAC layer did not
     * receive an acknowledgment. The MAC performs a datarate adaptation,
     * according to the LoRaWAN Specification V1.0.1, chapter 19.4, according
     * to the following table:
     *
     * Transmission nb | Data Rate
     * ----------------|-----------
     * 1 (first)       | DR
     * 2               | DR
     * 3               | max(DR-1,0)
     * 4               | max(DR-1,0)
     * 5               | max(DR-2,0)
     * 6               | max(DR-2,0)
     * 7               | max(DR-3,0)
     * 8               | max(DR-3,0)
     *
     * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
     * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
     */
    uint8_t NbTrials;
}McpsReqConfirmed_t;

/*!
 * LoRaMAC MCPS-Request for a proprietary frame
 */
typedef struct sMcpsReqProprietary
{
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
}McpsReqProprietary_t;

/*!
 * LoRaMAC MCPS-Request structure
 */
typedef struct sMcpsReq
{
    /*!
     * MCPS-Request type
     */
    Mcps_t Type;

    /*!
     * MCPS-Request parameters
     */
    union uMcpsParam
    {
        /*!
         * MCPS-Request parameters for an unconfirmed frame
         */
        McpsReqUnconfirmed_t Unconfirmed;
        /*!
         * MCPS-Request parameters for a confirmed frame
         */
        McpsReqConfirmed_t Confirmed;
        /*!
         * MCPS-Request parameters for a proprietary frame
         */
        McpsReqProprietary_t Proprietary;
    }Req;
}McpsReq_t;

/*!
 * LoRaMAC MCPS-Confirm
 */
typedef struct sMcpsConfirm
{
    /*!
     * Holds the previously performed MCPS-Request
     */
    Mcps_t McpsRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Uplink datarate
     */
    uint8_t Datarate;
    /*!
     * Transmission power
     */
    int8_t TxPower;
    /*!
     * Set if an acknowledgement was received
     */
    bool AckReceived;
    /*!
     * Provides the number of retransmissions
     */
    uint8_t NbRetries;
    /*!
     * The transmission time on air of the frame
     */
    TimerTime_t TxTimeOnAir;
    /*!
     * The uplink counter value related to the frame
     */
    uint32_t UpLinkCounter;
}McpsConfirm_t;

/*!
 * LoRaMAC MCPS-Indication primitive
 */
typedef struct sMcpsIndication
{
    /*!
     * MCPS-Indication type
     */
    Mcps_t McpsIndication;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Multicast
     */
    uint8_t Multicast;
    /*!
     * Application port
     */
    uint8_t Port;
    /*!
     * Downlink datarate
     */
    uint8_t RxDatarate;
    /*!
     * Frame pending status
     */
    uint8_t FramePending;
    /*!
     * Pointer to the received data stream
     */
    uint8_t *Buffer;
    /*!
     * Size of the received data stream
     */
    uint8_t BufferSize;
    /*!
     * Indicates, if data is available
     */
    bool RxData;
    /*!
     * Rssi of the received packet
     */
    int16_t Rssi;
    /*!
     * Snr of the received packet
     */
    uint8_t Snr;
    /*!
     * Receive window
     *
     * [0: Rx window 1, 1: Rx window 2]
     */
    uint8_t RxSlot;
    /*!
     * Set if an acknowledgement was received
     */
    bool AckReceived;
    /*!
     * The downlink counter value for the received frame
     */
    uint32_t DownLinkCounter;
}McpsIndication_t;

/*!
 * \brief LoRaMAC management services
 *
 * \details The following table list the primitives which are supported by the
 *          specific MAC management service:
 *
 * Name                  | Request | Indication | Response | Confirm
 * --------------------- | :-----: | :--------: | :------: | :-----:
 * \ref MLME_JOIN        | YES     | NO         | NO       | YES
 * \ref MLME_LINK_CHECK  | YES     | NO         | NO       | YES
 *
 * The following table provides links to the function implementations of the
 * related MLME primitives.
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MLME-Request     | \ref LoRaMacMlmeRequest
 * MLME-Confirm     | MacMlmeConfirm in \ref LoRaMacPrimitives_t
 */
typedef enum eMlme
{
    /*!
     * Initiates the Over-the-Air activation
     *
     * LoRaWAN Specification V1.0.1, chapter 6.2
     */
    MLME_JOIN,
    /*!
     * LinkCheckReq - Connectivity validation
     *
     * LoRaWAN Specification V1.0.1, chapter 5, table 4
     */
    MLME_LINK_CHECK,
}Mlme_t;

/*!
 * LoRaMAC MLME-Request for the join service
 */
typedef struct sMlmeReqJoin
{
    /*!
     * Globally unique end-device identifier
     *
     * LoRaWAN Specification V1.0.1, chapter 6.2.1
     */
    uint8_t *DevEui;
    /*!
     * Application identifier
     *
     * LoRaWAN Specification V1.0.1, chapter 6.1.2
     */
    uint8_t *AppEui;
    /*!
     * AES-128 application key
     *
     * LoRaWAN Specification V1.0.1, chapter 6.2.2
     */
    uint8_t *AppKey;
}MlmeReqJoin_t;

/*!
 * LoRaMAC MLME-Request structure
 */
typedef struct sMlmeReq
{
    /*!
     * MLME-Request type
     */
    Mlme_t Type;

    /*!
     * MLME-Request parameters
     */
    union uMlmeParam
    {
        /*!
         * MLME-Request parameters for a join request
         */
        MlmeReqJoin_t Join;
    }Req;
}MlmeReq_t;

/*!
 * LoRaMAC MLME-Confirm primitive
 */
typedef struct sMlmeConfirm
{
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t MlmeRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * The transmission time on air of the frame
     */
    TimerTime_t TxTimeOnAir;
    /*!
     * Demodulation margin. Contains the link margin [dB] of the last
     * successfully received LinkCheckReq
     */
    uint8_t DemodMargin;
    /*!
     * Number of gateways which received the last LinkCheckReq
     */
    uint8_t NbGateways;
}MlmeConfirm_t;

/*!
 * LoRa Mac Information Base (MIB)
 *
 * The following table lists the MIB parameters and the related attributes:
 *
 * Attribute                         | Get | Set
 * --------------------------------- | :-: | :-:
 * \ref MIB_DEVICE_CLASS             | YES | YES
 * \ref MIB_NETWORK_JOINED           | YES | YES
 * \ref MIB_ADR                      | YES | YES
 * \ref MIB_NET_ID                   | YES | YES
 * \ref MIB_DEV_ADDR                 | YES | YES
 * \ref MIB_NWK_SKEY                 | YES | YES
 * \ref MIB_APP_SKEY                 | YES | YES
 * \ref MIB_PUBLIC_NETWORK           | YES | YES
 * \ref MIB_REPEATER_SUPPORT         | YES | YES
 * \ref MIB_CHANNELS                 | YES | NO
 * \ref MIB_RX2_CHANNEL              | YES | YES
 * \ref MIB_CHANNELS_MASK            | YES | YES
 * \ref MIB_CHANNELS_NB_REP          | YES | YES
 * \ref MIB_MAX_RX_WINDOW_DURATION   | YES | YES
 * \ref MIB_RECEIVE_DELAY_1          | YES | YES
 * \ref MIB_RECEIVE_DELAY_2          | YES | YES
 * \ref MIB_JOIN_ACCEPT_DELAY_1      | YES | YES
 * \ref MIB_JOIN_ACCEPT_DELAY_2      | YES | YES
 * \ref MIB_CHANNELS_DATARATE        | YES | YES
 * \ref MIB_CHANNELS_DEFAULT_DATARATE| YES | YES
 * \ref MIB_CHANNELS_TX_POWER        | YES | YES
 * \ref MIB_UPLINK_COUNTER           | YES | YES
 * \ref MIB_DOWNLINK_COUNTER         | YES | YES
 * \ref MIB_MULTICAST_CHANNEL        | YES | NO
 *
 * The following table provides links to the function implementations of the
 * related MIB primitives:
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MIB-Set          | \ref LoRaMacMibSetRequestConfirm
 * MIB-Get          | \ref LoRaMacMibGetRequestConfirm
 */
typedef enum eMib
{
    /*!
     * LoRaWAN device class
     *
     * LoRaWAN Specification V1.0.1
     */
    MIB_DEVICE_CLASS,
    /*!
     * LoRaWAN Network joined attribute
     *
     * LoRaWAN Specification V1.0.1
     */
    MIB_NETWORK_JOINED,
    /*!
     * Adaptive data rate
     *
     * LoRaWAN Specification V1.0.1, chapter 4.3.1.1
     *
     * [true: ADR enabled, false: ADR disabled]
     */
    MIB_ADR,
    /*!
     * Network identifier
     *
     * LoRaWAN Specification V1.0.1, chapter 6.1.1
     */
    MIB_NET_ID,
    /*!
     * End-device address
     *
     * LoRaWAN Specification V1.0.1, chapter 6.1.1
     */
    MIB_DEV_ADDR,
    /*!
     * Network session key
     *
     * LoRaWAN Specification V1.0.1, chapter 6.1.3
     */
    MIB_NWK_SKEY,
    /*!
     * Application session key
     *
     * LoRaWAN Specification V1.0.1, chapter 6.1.4
     */
    MIB_APP_SKEY,
    /*!
     * Set the network type to public or private
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     *
     * [true: public network, false: private network]
     */
    MIB_PUBLIC_NETWORK,
    /*!
     * Support the operation with repeaters
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     *
     * [true: repeater support enabled, false: repeater support disabled]
     */
    MIB_REPEATER_SUPPORT,
    /*!
     * Communication channels. A get request will return a
     * pointer which references the first entry of the channel list. The
     * list is of size LORA_MAX_NB_CHANNELS
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_CHANNELS,
    /*!
     * Set receive window 2 channel
     *
     * LoRaWAN Specification V1.0.1, chapter 3.3.2
     */
    MIB_RX2_CHANNEL,
    /*!
     * LoRaWAN channels mask
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_CHANNELS_MASK,
    /*!
     * Set the number of repetitions on a channel
     *
     * LoRaWAN Specification V1.0.1, chapter 5.2
     */
    MIB_CHANNELS_NB_REP,
    /*!
     * Maximum receive window duration in [ms]
     *
     * LoRaWAN Specification V1.0.1, chapter 3.3.3
     */
    MIB_MAX_RX_WINDOW_DURATION,
    /*!
     * Receive delay 1 in [ms]
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_RECEIVE_DELAY_1,
    /*!
     * Receive delay 2 in [ms]
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_RECEIVE_DELAY_2,
    /*!
     * Join accept delay 1 in [ms]
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_JOIN_ACCEPT_DELAY_1,
    /*!
     * Join accept delay 2 in [ms]
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     */
    MIB_JOIN_ACCEPT_DELAY_2,
    /*!
     * Default Data rate of a channel
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     *
     * EU868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
     *
     * US915 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_8, DR_9, DR_10, DR_11, DR_12, DR_13]
     */
    MIB_CHANNELS_DEFAULT_DATARATE,
    /*!
     * Data rate of a channel
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     *
     * EU868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
     *
     * US915 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_8, DR_9, DR_10, DR_11, DR_12, DR_13]
     */
    MIB_CHANNELS_DATARATE,
    /*!
     * Transmission power of a channel
     *
     * LoRaWAN Specification V1.0.1, chapter 7
     *
     * EU868 - [TX_POWER_20_DBM, TX_POWER_14_DBM, TX_POWER_11_DBM,
     *          TX_POWER_08_DBM, TX_POWER_05_DBM, TX_POWER_02_DBM]
     *
     * US915 - [TX_POWER_30_DBM, TX_POWER_28_DBM, TX_POWER_26_DBM,
     *          TX_POWER_24_DBM, TX_POWER_22_DBM, TX_POWER_20_DBM,
     *          TX_POWER_18_DBM, TX_POWER_14_DBM, TX_POWER_12_DBM,
     *          TX_POWER_10_DBM]
     */
    MIB_CHANNELS_TX_POWER,
    /*!
     * LoRaWAN Up-link counter
     *
     * LoRaWAN Specification V1.0.1, chapter 4.3.1.5
     */
    MIB_UPLINK_COUNTER,
    /*!
     * LoRaWAN Down-link counter
     *
     * LoRaWAN Specification V1.0.1, chapter 4.3.1.5
     */
    MIB_DOWNLINK_COUNTER,
    /*!
     * Multicast channels. A get request will return a pointer to the first
     * entry of the multicast channel linked list. If the pointer is equal to
     * NULL, the list is empty.
     */
    MIB_MULTICAST_CHANNEL,

    MIB_DEV_EUI,
    MIB_APP_EUI,
    MIB_APP_KEY,
    MIB_DR_SCHM,
    MIB_PHY_TYPE,
    MIB_CUSTOM_RXWIN1,
    MIB_CHANNELS_NUM,
    MIB_POWER_DBM,
}Mib_t;

/*!
 * LoRaMAC MIB parameters
 */
typedef union uMibParam
{
    /*!
     * LoRaWAN device class
     *
     * Related MIB type: \ref MIB_DEVICE_CLASS
     */
    DeviceClass_t Class;
    /*!
     * LoRaWAN network joined attribute
     *
     * Related MIB type: \ref MIB_NETWORK_JOINED
     */
    bool IsNetworkJoined;
    /*!
     * Activation state of ADR
     *
     * Related MIB type: \ref MIB_ADR
     */
    bool AdrEnable;
    /*!
     * Network identifier
     *
     * Related MIB type: \ref MIB_NET_ID
     */
    uint32_t NetID;
    /*!
     * End-device address
     *
     * Related MIB type: \ref MIB_DEV_ADDR
     */
    uint32_t DevAddr;
    /*!
     * Network session key
     *
     * Related MIB type: \ref MIB_NWK_SKEY
     */
    uint8_t *NwkSKey;
    /*!
     * Application session key
     *
     * Related MIB type: \ref MIB_APP_SKEY
     */
    uint8_t *AppSKey;
    /*!
     * Enable or disable a public network
     *
     * Related MIB type: \ref MIB_PUBLIC_NETWORK
     */
    bool EnablePublicNetwork;
    /*!
     * Enable or disable repeater support
     *
     * Related MIB type: \ref MIB_REPEATER_SUPPORT
     */
    bool EnableRepeaterSupport;
    /*!
     * LoRaWAN Channel
     *
     * Related MIB type: \ref MIB_CHANNELS
     */
    ChannelParams_t* ChannelList;
     /*!
     * Channel for the receive window 2
     *
     * Related MIB type: \ref MIB_RX2_CHANNEL
     */
    Rx2ChannelParams_t Rx2Channel;
    /*!
     * Channel mask
     *
     * Related MIB type: \ref MIB_CHANNELS_MASK
     */
    uint16_t* ChannelsMask;
    /*!
     * Number of frame repetitions
     *
     * Related MIB type: \ref MIB_CHANNELS_NB_REP
     */
    uint8_t ChannelNbRep;
    /*!
     * Maximum receive window duration
     *
     * Related MIB type: \ref MIB_MAX_RX_WINDOW_DURATION
     */
    uint32_t MaxRxWindow;
    /*!
     * Receive delay 1
     *
     * Related MIB type: \ref MIB_RECEIVE_DELAY_1
     */
    uint32_t ReceiveDelay1;
    /*!
     * Receive delay 2
     *
     * Related MIB type: \ref MIB_RECEIVE_DELAY_2
     */
    uint32_t ReceiveDelay2;
    /*!
     * Join accept delay 1
     *
     * Related MIB type: \ref MIB_JOIN_ACCEPT_DELAY_1
     */
    uint32_t JoinAcceptDelay1;
    /*!
     * Join accept delay 2
     *
     * Related MIB type: \ref MIB_JOIN_ACCEPT_DELAY_2
     */
    uint32_t JoinAcceptDelay2;
    /*!
     * Channels data rate
     *
     * Related MIB type: \ref MIB_CHANNELS_DEFAULT_DATARATE
     */
    int8_t ChannelsDefaultDatarate;
    /*!
     * Channels data rate
     *
     * Related MIB type: \ref MIB_CHANNELS_DATARATE
     */
    int8_t ChannelsDatarate;
    /*!
     * Channels TX power
     *
     * Related MIB type: \ref MIB_CHANNELS_TX_POWER
     */
    int8_t ChannelsTxPower;
    /*!
     * LoRaWAN Up-link counter
     *
     * Related MIB type: \ref MIB_UPLINK_COUNTER
     */
    uint32_t UpLinkCounter;
    /*!
     * LoRaWAN Down-link counter
     *
     * Related MIB type: \ref MIB_DOWNLINK_COUNTER
     */
    uint32_t DownLinkCounter;
    /*!
     * Multicast channel
     *
     * Related MIB type: \ref MIB_MULTICAST_CHANNEL
     */
    MulticastParams_t* MulticastList;

    uint8_t *DevEui;
    uint8_t *AppEui;
    uint8_t *AppKey;
    struct{
        PhyType_t PhyType;
        uint8_t Dr;
        uint8_t Sf;
        uint16_t Bw;
        uint8_t Val;
    }DrSchm;
    PhyType_t PhyType;
    bool CustomRxWin1Flag;
    struct{
        uint32_t Freq;
        uint8_t Ch;
    }RxWin1;
    uint32_t ChNum;
    int8_t PowerDbm;
}MibParam_t;

/*!
 * LoRaMAC MIB-RequestConfirm structure
 */
typedef struct eMibRequestConfirm
{
    /*!
     * MIB-Request type
     */
    Mib_t Type;

    /*!
     * MLME-RequestConfirm parameters
     */
    MibParam_t Param;
}MibRequestConfirm_t;

/*!
 * LoRaMAC tx information
 */
typedef struct sLoRaMacTxInfo
{
    /*!
     * Defines the size of the applicative payload which can be processed
     */
    uint8_t MaxPossiblePayload;
    /*!
     * The current payload size, dependent on the current datarate
     */
    uint8_t CurrentPayloadSize;
}LoRaMacTxInfo_t;

/*!
 * LoRaMAC Status
 */
typedef enum eLoRaMacStatus
{
    /*!
     * Service started successfully
     */
    LORAMAC_STATUS_OK,
    /*!
     * Service not started - LoRaMAC is busy
     */
    LORAMAC_STATUS_BUSY,
    /*!
     * Service unknown
     */
    LORAMAC_STATUS_SERVICE_UNKNOWN,
    /*!
     * Service not started - invalid parameter
     */
    LORAMAC_STATUS_PARAMETER_INVALID,
    /*!
     * Service not started - invalid frequency
     */
    LORAMAC_STATUS_FREQUENCY_INVALID,
    /*!
     * Service not started - invalid datarate
     */
    LORAMAC_STATUS_DATARATE_INVALID,
    /*!
     * Service not started - invalid frequency and datarate
     */
    LORAMAC_STATUS_FREQ_AND_DR_INVALID,
    /*!
     * Service not started - the device is not in a LoRaWAN
     */
    LORAMAC_STATUS_NO_NETWORK_JOINED,
    /*!
     * Service not started - playload lenght error
     */
    LORAMAC_STATUS_LENGTH_ERROR,
    /*!
     * Service not started - playload lenght error
     */
    LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR,
    /*!
     * Service not started - the device is switched off
     */
    LORAMAC_STATUS_DEVICE_OFF,
    /*!
     * Already Joined a network
     */
    LORAMAC_STATUS_JOINED_ALREADY,
    /*!
     * Not in OTAA mode
     */
    LORAMAC_STATUS_NOT_OTAA_MODE,
    /*!
     * No channel is free
     */
    LORAMAC_STATUS_NO_FREE_CHANNEL,
    /*!
     * No channel is free
     */
    LORAMAC_STATUS_DATARATE_NOT_SUPPORTED,
    /*!
     * No channel is free
     */
    LORAMAC_STATUS_NO_BAND,
}LoRaMacStatus_t;

/*!
 * LoRaMAC events structure
 * Used to notify upper layers of MAC events
 */
typedef struct sLoRaMacPrimitives
{
    /*!
     * \brief   MCPS-Confirm primitive
     *
     * \param   [OUT] MCPS-Confirm parameters
     */
    void ( *MacMcpsConfirm )( McpsConfirm_t *McpsConfirm );
    /*!
     * \brief   MCPS-Indication primitive
     *
     * \param   [OUT] MCPS-Indication parameters
     */
    void ( *MacMcpsIndication )( McpsIndication_t *McpsIndication );
    /*!
     * \brief   MLME-Confirm primitive
     *
     * \param   [OUT] MLME-Confirm parameters
     */
    void ( *MacMlmeConfirm )( MlmeConfirm_t *MlmeConfirm );
}LoRaMacPrimitives_t;

typedef struct sLoRaMacCallback
{
    /*!
     * \brief   Measures the battery level
     *
     * \retval  Battery level [0: node is connected to an external
     *          power source, 1..254: battery level, where 1 is the minimum
     *          and 254 is the maximum value, 255: the node was not able
     *          to measure the battery level]
     */
    uint8_t ( *GetBatteryLevel )( void );
}LoRaMacCallback_t;

/*!
 * \brief   LoRaMAC layer initialization
 *
 * \details In addition to the initialization of the LoRaMAC layer, this
 *          function initializes the callback primitives of the MCPS and
 *          MLME services. Every data field of \ref LoRaMacPrimitives_t must be
 *          set to a valid callback function.
 *
 * \param   [IN] events - Pointer to a structure defining the LoRaMAC
 *                        event functions. Refer to \ref LoRaMacPrimitives_t.
 *
 * \param   [IN] events - Pointer to a structure defining the LoRaMAC
 *                        callback functions. Refer to \ref LoRaMacCallback_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacInitialization( PhyType_t type, LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks );

/*!
 * \brief   Queries the LoRaMAC if it is possible to send the next frame with
 *          a given payload size. The LoRaMAC takes scheduled MAC commands into
 *          account and reports, when the frame can be send or not.
 *
 * \param   [IN] size - Size of applicative payload to be send next
 *
 * \param   [OUT] txInfo - The structure \ref LoRaMacTxInfo_t contains
 *                         information about the actual maximum payload possible
 *                         ( according to the configured datarate or the next
 *                         datarate according to ADR ), and the maximum frame
 *                         size, taking the scheduled MAC commands into account.
 *
 * \retval  LoRaMacStatus_t Status of the operation. When the parameters are
 *          not valid, the function returns \ref LORAMAC_STATUS_PARAMETER_INVALID.
 *          In case of a length error caused by the applicative payload size, the
 *          function returns LORAMAC_STATUS_LENGTH_ERROR. In case of a length error
 *          due to additional MAC commands in the queue, the function returns
 *          LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR. In case the query is valid, and
 *          the LoRaMAC is able to send the frame, the function returns LORAMAC_STATUS_OK. *
 */
LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo );

/*!
 * \brief   LoRaMAC channel add service
 *
 * \details Adds a new channel to the channel list and activates the id in
 *          the channel mask. For the US915 band, all channels are enabled
 *          by default. It is not possible to activate less than 6 125 kHz
 *          channels.
 *
 * \param   [IN] id - Id of the channel. Possible values are:
 *
 *          0-15 for EU868
 *          0-72 for US915
 *
 * \param   [IN] params - Channel parameters to set.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params );

/*!
 * \brief   LoRaMAC channel remove service
 *
 * \details Deactivates the id in the channel mask.
 *
 * \param   [IN] id - Id of the channel.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id );

/*!
 * \brief   LoRaMAC multicast channel link service
 *
 * \details Links a multicast channel into the linked list.
 *
 * \param   [IN] channelParam - Multicast channel parameters to link.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam );

/*!
 * \brief   LoRaMAC multicast channel unlink service
 *
 * \details Unlinks a multicast channel from the linked list.
 *
 * \param   [IN] channelParam - Multicast channel parameters to unlink.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam );

/*!
 * \brief   LoRaMAC MIB-Get
 *
 * \details The mac information base service to get attributes of the LoRaMac
 *          layer.
 *
 *          The following code-snippet shows how to use the API to get the
 *          parameter AdrEnable, defined by the enumeration type
 *          \ref MIB_ADR.
 * \code
 * MibRequestConfirm_t mibReq;
 * mibReq.Type = MIB_ADR;
 *
 * if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
 * {
 *   // LoRaMAC updated the parameter mibParam.AdrEnable
 * }
 * \endcode
 *
 * \param   [IN] mibRequest - MIB-GET-Request to perform. Refer to \ref MibRequestConfirm_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet );

/*!
 * \brief   LoRaMAC MIB-Set
 *
 * \details The mac information base service to set attributes of the LoRaMac
 *          layer.
 *
 *          The following code-snippet shows how to use the API to set the
 *          parameter AdrEnable, defined by the enumeration type
 *          \ref MIB_ADR.
 *
 * \code
 * MibRequestConfirm_t mibReq;
 * mibReq.Type = MIB_ADR;
 * mibReq.Param.AdrEnable = true;
 *
 * if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
 * {
 *   // LoRaMAC updated the parameter
 * }
 * \endcode
 *
 * \param   [IN] mibRequest - MIB-SET-Request to perform. Refer to \ref MibRequestConfirm_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet );

/*!
 * \brief   LoRaMAC MLME-Request
 *
 * \details The Mac layer management entity handles management services. The
 *          following code-snippet shows how to use the API to perform a
 *          network join request.
 *
 * \code
 * static uint8_t DevEui[] =
 * {
 *   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * static uint8_t AppEui[] =
 * {
 *   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * static uint8_t AppKey[] =
 * {
 *   0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
 *   0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
 * };
 *
 * MlmeReq_t mlmeReq;
 * mlmeReq.Type = MLME_JOIN;
 * mlmeReq.Req.Join.DevEui = DevEui;
 * mlmeReq.Req.Join.AppEui = AppEui;
 * mlmeReq.Req.Join.AppKey = AppKey;
 *
 * if( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK )
 * {
 *   // Service started successfully. Waiting for the Mlme-Confirm event
 * }
 * \endcode
 *
 * \param   [IN] mlmeRequest - MLME-Request to perform. Refer to \ref MlmeReq_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID,
 *          \ref LORAMAC_STATUS_NO_NETWORK_JOINED,
 *          \ref LORAMAC_STATUS_LENGTH_ERROR,
 *          \ref LORAMAC_STATUS_DEVICE_OFF.
 */
LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest );

/*!
 * \brief   LoRaMAC MCPS-Request
 *
 * \details The Mac Common Part Sublayer handles data services. The following
 *          code-snippet shows how to use the API to send an unconfirmed
 *          LoRaMAC frame.
 *
 * \code
 * uint8_t myBuffer[] = { 1, 2, 3 };
 *
 * McpsReq_t mcpsReq;
 * mcpsReq.Type = MCPS_UNCONFIRMED;
 * mcpsReq.Req.Unconfirmed.fPort = 1;
 * mcpsReq.Req.Unconfirmed.fBuffer = myBuffer;
 * mcpsReq.Req.Unconfirmed.fBufferSize = sizeof( myBuffer );
 *
 * if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
 * {
 *   // Service started successfully. Waiting for the MCPS-Confirm event
 * }
 * \endcode
 *
 * \param   [IN] mcpsRequest - MCPS-Request to perform. Refer to \ref McpsReq_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID,
 *          \ref LORAMAC_STATUS_NO_NETWORK_JOINED,
 *          \ref LORAMAC_STATUS_LENGTH_ERROR,
 *          \ref LORAMAC_STATUS_DEVICE_OFF.
 */
LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest );

typedef struct{
    LoRaMacHeader_t mhdr;
    uint32_t devaddr;
    LoRaMacFrameCtrl_t fctrl;
    uint32_t fcnt;
    uint8_t *fopts;
    uint8_t fport;
}LoRaMacExPackPara_t;

int LoRaMacExPack(uint8_t *buf, uint8_t *pl, int len, LoRaMacExPackPara_t *para);

/*!
 * Datarate numbers
 */
#define LORA_MAX_DR                                 (16)

/*!
 * Maximum channels for all bands
 */
#define LORA_MAX_CHS                                (96)

/*!
 * Maximum bands
 */
#define LORA_MAX_BANDS                              (5)

#ifndef USE_DEBUGGER
#define LORAMAC_PIN_INIT()
#define LORAMAC_PIN_SET()
#define LORAMAC_PIN_CLR()
#else
#define LORAMAC_PIN_INIT()                          { Gpio_t gpio;\
                                                      GpioInit( &gpio, PB_12, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 ); \
                                                      GpioWrite( &gpio, 0);}
#define LORAMAC_PIN_SET()                           (GPIOB->BSRR = (1<<12))
#define LORAMAC_PIN_CLR()                           (GPIOB->BRR = (1<<12))
#endif

typedef struct{
    //PhyType_t type;
    Rx2ChannelParams_t rxwin2;
    struct{
        uint8_t num;
        const ChannelParams_t *buf;
        uint8_t size;
    }ch;
    const uint8_t *drtab;
    struct{
        uint8_t MaxEIRPIndex;
        uint8_t TxPowerMax;
    }power;
    struct{
        uint8_t size;
        const uint8_t *buf;
    }drpllen;
    const uint16_t *chmsk;
}RegionPara_t;

extern int8_t LORA_MAX_NB_CHANNELS;
extern int8_t LORA_MAX_NB_BANDS;

extern int8_t LORAMAC_DEFAULT_TX_POWER;
extern int8_t LORAMAC_MAX_TX_POWER;
extern int8_t LORAMAC_MIN_TX_POWER;

extern int8_t LORAMAC_DEFAULT_DATARATE;
extern int8_t LORAMAC_TX_MIN_DATARATE;
extern int8_t LORAMAC_TX_MAX_DATARATE;
extern int8_t LORAMAC_RX_MIN_DATARATE;
extern int8_t LORAMAC_RX_MAX_DATARATE;

extern int8_t LORAMAC_MIN_RX1_DR_OFFSET;
extern int8_t LORAMAC_MAX_RX1_DR_OFFSET;

extern int16_t LORAMAC_RSSI_THRESH;

extern uint32_t LORAMAC_FIRST_RX1_CHANNEL;
extern uint32_t LORAMAC_LAST_RX1_CHANNEL;
extern uint8_t LORAMAC_PERODIC_RX1_CHANNEL;
extern uint32_t LORAMAC_STEPWIDTH_RX1_CHANNEL;

extern uint8_t LORAMAC_START_CH_ID;

extern uint8_t LORA_MAC_COMMAND_MAX_LENGTH;

extern int8_t LORA_RXWIN_OFFSET;

extern bool MacCommandsInNextTx;
extern uint8_t MacCommandsBufferIndex;
extern uint8_t MacCommandsBufferToRepeatIndex;
extern uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_BUF_SIZE];
extern uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_BUF_SIZE];

extern TimerTime_t LoRaMacBandTimeToValid;
extern uint16_t AggregatedDCycle;

extern ChannelParams_t Channels[LORA_MAX_CHS];
extern uint16_t ChannelsMask[6];
extern uint16_t ChannelsMaskDefault[6];
extern Rx2ChannelParams_t Rx2Channel;
extern int8_t ChannelsDatarate;
extern int8_t ChannelsDefaultDatarate;

extern uint32_t ReceiveDelay1;
extern uint32_t ReceiveDelay2;
extern uint32_t JoinAcceptDelay1;
extern uint32_t JoinAcceptDelay2;

extern DeviceClass_t LoRaMacDeviceClass;
extern bool PublicNetwork;
extern uint8_t ChannelsNbRep;
extern bool AdrCtrlOn;
extern int8_t ChannelsTxPower;

extern uint32_t LoRaMacDevAddr;
extern uint8_t LoRaMacDevEui[8];
extern uint8_t LoRaMacAppEui[8];
extern uint8_t LoRaMacAppKey[16];
extern uint8_t LoRaMacNwkSKey[16];
extern uint8_t LoRaMacAppSKey[16];
extern uint16_t LoRaMacDevNonce;
extern uint32_t LoRaMacNetID;

extern bool DownLinkCounterCheckingRelaxed;
extern uint32_t UpLinkCounter;
extern uint32_t DownLinkCounter;
extern bool IsLoRaMacNetworkJoined;
extern bool AdrCtrlOn;

typedef struct{
    bool flag;
    struct{
        uint32_t start;
        uint32_t done;
    }tx;
    struct{
        uint32_t start;
        uint32_t done;
    }rx1;
    struct{
        uint32_t start;
        uint32_t done;
    }rx2;
}LoRaMacTiming_t;

extern LoRaMacTiming_t LoRaMacTiming;

typedef union{
    uint8_t Value;
    struct
    {
        int8_t Power                : 7;
        uint8_t Flag                : 1;
    }Fields;
}ForceTxPower_t;

typedef union{
    uint8_t Value;
    struct
    {
        uint8_t MaxEirp             : 4;
        uint8_t UplinkDwellTime     : 1;
        uint8_t DownlinkDwellTime   : 1;
        uint8_t RFU                 : 2;
    }Fields;
}EirpDwellTime_t;
extern EirpDwellTime_t EirpDwellTime;

extern PhyType_t PhyType;

extern int8_t TxPowers[LORA_MAX_DR];

extern ForceTxPower_t ForceTxPower;

extern bool LoRaMacJoinDutyCycle;

LoRaMacStatus_t LoRaMacCheck(void);

void LoRaMacExInit( PhyType_t phytype );

bool LoRaMacExSetDwellTime(bool ul, bool dl);

int LoRaMacExSetMaxEirp(uint8_t index);

/*! \} defgroup LORAMAC */

#endif // __LORAMAC_H__
