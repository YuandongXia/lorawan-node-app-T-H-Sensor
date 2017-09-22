/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jäckle ( STACKFORCE )
*/
#include "board.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"

#ifdef USE_DEBUGGER
LoRaMacTiming_t LoRaMacTiming;
#define LORAMAC_TIMING_TX_START() \
    do { \
        LoRaMacTiming.flag = false; \
        LoRaMacTiming.tx.start = TimerGetCurrentTime(); \
        LoRaMacTiming.tx.done = 0; \
        LoRaMacTiming.rx1.start = 0; \
        LoRaMacTiming.rx1.done = 0; \
        LoRaMacTiming.rx2.start = 0; \
        LoRaMacTiming.rx2.done = 0; \
    }while(0)
#define LORAMAC_TIMING_TX_DONE() \
    do { \
        LoRaMacTiming.tx.done = TimerGetCurrentTime(); \
    }while(0)

#define LORAMAC_TIMING_RX1_START() \
    do { \
        LoRaMacTiming.rx1.start = TimerGetCurrentTime(); \
    }while(0)
#define LORAMAC_TIMING_RX2_START() \
    do { \
        LoRaMacTiming.rx2.start = TimerGetCurrentTime(); \
    }while(0)
#define LORAMAC_TIMING_RX_DONE() \
    do { \
        if( RxSlot == 0 ){ \
            LoRaMacTiming.rx1.done = TimerGetCurrentTime(); \
        } else { \
            LoRaMacTiming.rx2.done = TimerGetCurrentTime(); \
        } \
    }while(0)


#else

#define LORAMAC_TIMING_TX_START()
#define LORAMAC_TIMING_TX_DONE()
#define LORAMAC_TIMING_RX1_START()
#define LORAMAC_TIMING_RX2_START()
#define LORAMAC_TIMING_RX_DONE()

#endif

/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Fixed constant of MAX FOPTS command
 */
#define LORA_MAC_COMMAND_FOPTS_MAX_LENGTH           15

#define US915_TX_POWER_26_DBM                       (2)
#define US915_TX_POWER_20_DBM                       (5)

/*!
 * FRMPayload overhead to be used when setting the Radio.SetMaxPayloadLength
 * in RxWindowSetup function.
 * Maximum PHYPayload = MaxPayloadOfDatarate/MaxPayloadOfDatarateRepeater + LORA_MAC_FRMPAYLOAD_OVERHEAD
 */
#define LORA_MAC_FRMPAYLOAD_OVERHEAD                12 // MHDR(1) + FHDR(7) + MIC(4)

/*!
 * Join Duty Cycle Limitation
 */
#define BACKOFF_DC_1_HOUR       100
#define BACKOFF_DC_10_HOURS     1000
#define BACKOFF_DC_24_HOURS     10000

/*!
 * Device IEEE EUI
 */
uint8_t LoRaMacDevEui[8];

/*!
 * Application IEEE EUI
 */
uint8_t LoRaMacAppEui[8];

/*!
 * AES encryption/decryption cipher application key
 */
uint8_t LoRaMacAppKey[16];

/*!
 * AES encryption/decryption cipher network session key
 */
uint8_t LoRaMacNwkSKey[16];

/*!
 * AES encryption/decryption cipher application session key
 */
uint8_t LoRaMacAppSKey[16];

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
uint32_t LoRaMacDevAddr;

/*!
 * Multicast channels linked list
 */
MulticastParams_t *MulticastChannels;

/*!
 * Actual device class
 */
DeviceClass_t LoRaMacDeviceClass;

/*!
 * Indicates if the node is connected to a private or public network
 */
bool PublicNetwork;

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen;

/*!
 * Buffer containing the upper layer data.
 */
static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Check Downlink Counter Relaxed
 */
bool DownLinkCounterCheckingRelaxed;

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
uint32_t UpLinkCounter;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
uint32_t DownLinkCounter;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
bool IsLoRaMacNetworkJoined;

/*!
 * LoRaMac ADR control status
 */
bool AdrCtrlOn;

/*!
 * Counts the number of missed ADR acknowledgements
 */
static uint32_t AdrAckCounter;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
bool MacCommandsInNextTx;

/*!
 * Contains the current MacCommandsBuffer index
 */
uint8_t MacCommandsBufferIndex;

/*!
 * Contains the current MacCommandsBuffer index for MAC commands to repeat
 */
uint8_t MacCommandsBufferToRepeatIndex;

/*!
 * Buffer containing the MAC layer commands
 */
uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_BUF_SIZE];

/*!
 * Buffer containing the MAC layer commands which must be repeated
 */
uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_BUF_SIZE];

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
static uint8_t MaxPayloadOfDatarate[LORA_MAX_DR];
static uint8_t DlMaxPayloadOfDatarate[LORA_MAX_DR];

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
int8_t TxPowers[LORA_MAX_DR];

/*!
 * LoRaMAC channels
 */
ChannelParams_t Channels[LORA_MAX_CHS];

static Band_t Bands[LORA_MAX_BANDS];

PhyType_t PhyType;

const RegionPara_t *Region;

/*!
 * LoRaMAC 2nd reception window settings
 */
Rx2ChannelParams_t Rx2Channel;

/*!
 * Channels Tx output power
 */
int8_t ChannelsTxPower;

/*!
 * Channels datarate
 */
int8_t ChannelsDatarate;

/*!
 * Channels default datarate
 */
int8_t ChannelsDefaultDatarate;

/*!
 * Datarate offset between uplink and downlink on first window
 */
static uint8_t Rx1DrOffset;

/*!
 * Contains the channels which remain to be applied.
 */
static uint16_t ChannelsMaskRemaining[6];
uint16_t ChannelsMaskDefault[6];

ForceTxPower_t ForceTxPower;
bool LoRaMacJoinDutyCycle;

int8_t LORA_RXWIN_OFFSET = 0;

/*!
 * LORAMAC Macros
 */
int8_t LORA_MAX_NB_CHANNELS;
int8_t LORA_MAX_NB_BANDS;

int8_t LORAMAC_DEFAULT_TX_POWER;
int8_t LORAMAC_MAX_TX_POWER;
int8_t LORAMAC_MIN_TX_POWER;

int8_t LORAMAC_DEFAULT_DATARATE;
int8_t LORAMAC_TX_MIN_DATARATE;
int8_t LORAMAC_TX_MAX_DATARATE;
int8_t LORAMAC_RX_MIN_DATARATE;
int8_t LORAMAC_RX_MAX_DATARATE;

int8_t LORAMAC_MIN_RX1_DR_OFFSET;
int8_t LORAMAC_MAX_RX1_DR_OFFSET;

int16_t LORAMAC_RSSI_THRESH;

uint32_t LORAMAC_FIRST_RX1_CHANNEL;
uint32_t LORAMAC_LAST_RX1_CHANNEL;
uint8_t LORAMAC_PERODIC_RX1_CHANNEL;
uint32_t LORAMAC_STEPWIDTH_RX1_CHANNEL;

uint8_t LORAMAC_START_CH_ID;

uint8_t LORA_MAC_COMMAND_MAX_LENGTH;

const uint8_t Eu868MaxPayloadOfDatarate[] = { 51+1, 51+1, 51+1, 115+1, 242+1, 242+1, 242+1, 242+1 };

const uint8_t Cn470MaxPayloadOfDatarate[] = { 51+1, 51+1, 51+1, 115+1, 242+1, 242+1 };

const uint8_t Us915MaxPayloadOfDatarate[] = { 11+1, 53+1, 125+1, 242+1, 242+1, 0, 0, 0, 53+1, 129+1, 242+1, 242+1, 242+1, 242+1 };

const uint8_t Au915MaxPayloadOfDatarate[] = { 51+1, 51+1, 51+1, 115+1, 242+1, 242+1, 0, 53+1, 129+1, 242+1, 242+1, 242+1, 242+1 };

const uint8_t As923MaxPayloadOfDatarateDwell[] = { 0, 0, 11+1, 53+1, 125+1, 242+1, 242+1, 242+1 };

const uint8_t Kr920MaxPayloadOfDatarate[] = { 65+1, 151+1, 242+1, 242+1, 242+1, 242+1};

int DlChannel[LORA_MAX_CHS];

const int8_t MaxEirpTab[16] = {
    8, 10, 12, 13, 14, 16, 18, 20, 21, 24, 26, 27, 29, 30, 33, 36
};

EirpDwellTime_t EirpDwellTime;

typedef enum {
    BAND_UNKNOWN = -1,
    BAND_G1_0    = 0,
    BAND_G1_1    = 1,
    BAND_G1_2    = 2,
    BAND_G1_3    = 3,
    BAND_G1_4    = 4,
}BandId_t;

TimerTime_t LoRaMacBandTimeToValid;

static const uint8_t *LoRaMacDrTab;           // save sf and bw

const uint8_t LoRaMacEu868DrTab[LORA_MAX_DR]={
    DR(SF12, BW125),    // DR0
    DR(SF11, BW125),    // DR1
    DR(SF10, BW125),    // DR2
    DR(SF9, BW125),     // DR3
    DR(SF8, BW125),     // DR4
    DR(SF7, BW125),     // DR5
    DR(SF7, BW250),     // DR6
    DR(FSK, BW125),     // DR7
    DR_RFU,             // DR8
    DR_RFU,             // DR9
    DR_RFU,             // DR10
    DR_RFU,             // DR11
    DR_RFU,             // DR12
    DR_RFU,             // DR13
    DR_RFU,             // DR14
    DR_RFU,             // DR15
};

const uint8_t LoRaMacAu915DrTab[LORA_MAX_DR]={
    DR(SF12, BW125),    // DR0
    DR(SF11, BW125),    // DR1
    DR(SF10, BW125),    // DR2
    DR(SF9, BW125),     // DR3
    DR(SF8, BW125),     // DR4
    DR(SF7, BW125),     // DR5
    DR(SF8, BW500),     // DR6
    DR_RFU,             // DR7
    DR(SF12, BW500),    // DR8
    DR(SF11, BW500),    // DR9
    DR(SF10, BW500),    // DR10
    DR(SF9, BW500),     // DR11
    DR(SF8, BW500),     // DR12
    DR(SF7, BW500),     // DR13
    DR_RFU,             // DR14
    DR_RFU,             // DR15
};

const uint8_t LoRaMacUs915DrTab[LORA_MAX_DR]={
    DR(SF10, BW125),    // DR0
    DR(SF9, BW125),     // DR1
    DR(SF8, BW125),     // DR2
    DR(SF7, BW125),     // DR3
    DR(SF8, BW500),     // DR4
    DR_RFU,             // DR5
    DR_RFU,             // DR6
    DR_RFU,             // DR7
    DR(SF12, BW500),    // DR8
    DR(SF11, BW500),    // DR9
    DR(SF10, BW500),    // DR10
    DR(SF9, BW500),     // DR11
    DR(SF8, BW500),     // DR12
    DR(SF7, BW500),     // DR13
    DR_RFU,             // DR14
    DR_RFU,             // DR15
};

const uint8_t LoRaMacCn470DrTab[LORA_MAX_DR]={
    DR(SF12, BW125),    // DR0
    DR(SF11, BW125),    // DR1
    DR(SF10, BW125),    // DR2
    DR(SF9, BW125),     // DR3
    DR(SF8, BW125),     // DR4
    DR(SF7, BW125),     // DR5
    DR_RFU,             // DR6
    DR_RFU,             // DR7
    DR_RFU,             // DR8
    DR_RFU,             // DR9
    DR_RFU,             // DR10
    DR_RFU,             // DR11
    DR_RFU,             // DR12
    DR_RFU,             // DR13
    DR_RFU,             // DR14
    DR_RFU,             // DR15
};

/******************************************************************************/
// LoRaMac Extension API

void LoRaMacExInit( PhyType_t phytype );

int LoRaMacExCheckFreq(uint32_t freq);

bool LoRaMacExRx2FreqInRange( uint32_t freq );

uint32_t LoRaMacExGetRx1Freq(int ch);

void LoRaMacExEnableDefaultChannels(void);

void LoRaMacExUpdateMacCommandMaxLen(uint8_t datarate);

uint16_t LoRaMacExGetRxWinSymbols(uint8_t sf, uint8_t bw);

int8_t LoRaMacExGetRxWinTimeOffset(uint8_t dr_val);

int8_t LoRaMacExSetRx1DrOft(int8_t dr);

int8_t LoRaMacExLimitPower(int8_t power);

bool LoRaMacExCheckSrvMacCommand(uint8_t *cmd, uint8_t len);

/******************************************************************************/

/*!
 * Mask indicating which channels are enabled
 */
uint16_t ChannelsMask[6];

/*!
 * Number of uplink messages repetitions [1:15] (unconfirmed messages only)
 */
uint8_t ChannelsNbRep;

/*!
 * Uplink messages repetitions counter
 */
static uint8_t ChannelsNbRepCounter;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device.
 */
static uint8_t MaxDCycle;

/*!
 * Aggregated duty cycle management
 */
uint16_t AggregatedDCycle;
static TimerTime_t AggregatedLastTxDoneTime;
static TimerTime_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
static bool DutyCycleOn;

/*!
 * Current channel index
 */
static uint8_t Channel;

/*!
 * Stores the time at LoRaMac initialization.
 *
 * \remark Used for the BACKOFF_DC computation.
 */
static TimerTime_t LoRaMacInitializationTime = 0;

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState
{
    MAC_IDLE          = 0x00000000,
    MAC_TX_RUNNING    = 0x00000001,
    MAC_RX            = 0x00000002,
    MAC_ACK_REQ       = 0x00000004,
    MAC_ACK_RETRY     = 0x00000008,
    MAC_TX_DELAYED    = 0x00000010,
    MAC_TX_CONFIG     = 0x00000020,
    MAC_RX_ABORT      = 0x00000040,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
static TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacPrimitives_t *LoRaMacPrimitives;

/*!
 * LoRaMac upper layer callback functions
 */
static LoRaMacCallback_t *LoRaMacCallbacks;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
static TimerEvent_t TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay from end of Tx
 */
uint32_t ReceiveDelay1;
uint32_t ReceiveDelay2;
uint32_t JoinAcceptDelay1;
uint32_t JoinAcceptDelay2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * LoRaMac maximum time a reception window stays open
 */
static uint32_t MaxRxWindow;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
static bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
TimerTime_t TxTimeOnAir = 0;

/*!
 * Number of trials for the Join Request
 */
static uint16_t JoinRequestTrials;

/*!
 * Structure to hold an MCPS indication data.
 */
static McpsIndication_t McpsIndication;

/*!
 * Structure to hold MCPS confirm data.
 */
static McpsConfirm_t McpsConfirm;

/*!
 * Structure to hold MLME confirm data.
 */
static MlmeConfirm_t MlmeConfirm;

/*!
 * Holds the current rx window slot
 */
static uint8_t RxSlot;

/*!
 * LoRaMac tx/rx operation state
 */
LoRaMacFlags_t LoRaMacFlags;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * \brief This function prepares the MAC to abort the execution of function
 *        OnRadioRxDone in case of a reception error.
 */
static void PrepareRxDoneAbort( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

/*!
 * \brief Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void );

/*!
 * \brief Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void );

/*!
 * \brief Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * \brief Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Searches and set the next random available channel
 *
 * \param [OUT] Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \retval status  Function status [1: OK, 0: Unable to find a channel on the
 *                                  current datarate]
 */
static LoRaMacStatus_t SetNextChannel( TimerTime_t* time );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \param [IN] enable if true, it enables a public network
 */
void SetPublicNetwork( bool enable );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 */
static void RxWindowSetup( uint32_t freq, int8_t datarate, uint8_t drval, bool rxContinuous );

/*!
 * \brief Verifies if the RX window 2 frequency is in range
 *
 * \param [IN] freq window channel frequency
 *
 * \retval status  Function status [1: OK, 0: Frequency not applicable]
 */
//static bool Rx2FreqInRange( uint32_t freq );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 );

/*!
 * \brief Parses the MAC commands which must be repeated.
 *
 * \Remark MAC layer internal function
 *
 * \param [IN] cmdBufIn  Buffer which stores the MAC commands to send
 * \param [IN] length  Length of the input buffer to parse
 * \param [OUT] cmdBufOut  Buffer which stores the MAC commands which must be
 *                         repeated.
 *
 * \retval Size of the MAC commands to repeat.
 */
static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut );

/*!
 * \brief Validates if the payload fits into the frame, taking the datarate
 *        into account.
 *
 * \details Refer to chapter 4.3.2 of the LoRaWAN specification, v1.0
 *
 * \param lenN Length of the application payload. The length depends on the
 *             datarate and is region specific
 *
 * \param datarate Current datarate
 *
 * \param fOptsLen Length of the fOpts field
 *
 * \retval [false: payload does not fit into the frame, true: payload fits into
 *          the frame]
 */
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen );

/*!
 * \brief Counts the number of bits in a mask.
 *
 * \param [IN] channelsMask A mask pointer from which the function counts the active bits.
 * \param [IN] channelsMaskNum channelsMask numbers to count.
 *
 * \retval Number of enabled bits in the mask.
 */
uint8_t CountChannelMask(uint16_t *channelsMask, uint8_t channelsMaskNum);

/*****************************************************************************/
// US915 only APIs

/*!
 * \brief Counts the number of enabled 125 kHz channels in the channel mask.
 *        This function can only be applied to US915 band.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval Number of enabled channels in the channel mask
 */
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask );

/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] mask Block definition to set.
 * \param [OUT] channelsMask Pointer to the first element of the channel mask
 */
//static void ReenableChannels( uint16_t mask, uint16_t* channelMask );

/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval [true: channel mask correct, false: channel mask not correct]
 */
bool ValidateChannelsMask( uint16_t* channelMask );

/*****************************************************************************/

/*!
 * \brief Limits the Tx power according to the number of enabled channels
 *
 * \retval Returns the maximum valid tx power
 */
static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower );

/*!
 * \brief Verifies, if a value is in a given range.
 *
 * \param value Value to verify, if it is in range
 *
 * \param min Minimum possible value
 *
 * \param max Maximum possible value
 *
 * \retval Returns the maximum valid tx power
 */
static bool ValueInRange( int8_t value, int8_t min, int8_t max );

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off
 *
 * \param [IN] adrEnabled Specify whether ADR is on or off
 *
 * \param [IN] updateChannelMask Set to true, if the channel masks shall be updated
 *
 * \param [OUT] datarateOut Reports the datarate which will be used next
 *
 * \retval Returns the state of ADR ack request
 */
static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut );

/*!
 * \brief Disables channel in a specified channel mask
 *
 * \param [IN] id - Id of the channel
 *
 * \param [IN] mask - Pointer to the channel mask to edit
 *
 * \retval [true, if disable was successful, false if not]
 */
static bool DisableChannelInMask( uint8_t id, uint16_t* mask );

/*!
 * \brief Decodes MAC commands in the fOpts field and in the payload
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr );

/*!
 * \brief LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * \brief LoRaMAC layer frame buffer initialization
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*
 * \brief Schedules the frame according to the duty cycle
 *
 * \retval Status of the operation
 */
static LoRaMacStatus_t ScheduleTx( void );

/*
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

/*
 * \brief Alternates the datarate of the channel for the join request.
 *
 * \param [IN] nbTrials    Number of performed join requests.
 * \retval Datarate to apply
 */
static int8_t AlternateDatarate( uint16_t nbTrials );

/*!
 * \brief LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark PrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel );

uint16_t ChannelDrMap(uint16_t *chmsk);
uint8_t DataRateFormat(uint16_t drmap, uint8_t datarate);

static void OnRadioTxDone( void )
{
    LORAMAC_PIN_CLR();
    LORAMAC_TIMING_TX_DONE();

    TimerTime_t curTime = TimerGetCurrentTime( );
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
    TimerStart( &RxWindowTimer1 );
    if( LoRaMacDeviceClass != CLASS_C )
    {
        TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
        TimerStart( &RxWindowTimer2 );
    }
    if( ( LoRaMacDeviceClass == CLASS_C ) || ( NodeAckRequested == true ) )
    {
        TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + ACK_TIMEOUT +
                                         randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
        TimerStart( &AckTimeoutTimer );
    }

    if( NodeAckRequested == false )
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        ChannelsNbRepCounter++;
    }

    // Store last Tx channel
    // Update last tx done time for the current channel
    Bands[Channels[Channel].Band].LastTxDoneTime = curTime;
    // Update Aggregated last tx done time
    AggregatedLastTxDoneTime = curTime;
    CalculateBackOff( Channel );
}

static void PrepareRxDoneAbort( void )
{
    LoRaMacState |= MAC_RX_ABORT;

    if( ( NodeAckRequested ) || ( LoRaMacDeviceClass == CLASS_C ) )
    {
        OnAckTimeoutTimerEvent( );
    }

    LoRaMacFlags.Bits.McpsInd = 1;
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );
    TimerStart( &MacStateCheckTimer );
}

static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    LORAMAC_PIN_CLR();

    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    //bool skipIndication = false;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;

    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    MulticastParams_t *curMulticastParams = NULL;
    uint8_t *nwkSKey = LoRaMacNwkSKey;
    uint8_t *appSKey = LoRaMacAppSKey;

    uint8_t multicast = 0;

    bool isMicOk = false;

    McpsConfirm.AckReceived = false;
    McpsIndication.Rssi = rssi;
    McpsIndication.Snr = snr;
    McpsIndication.RxSlot = RxSlot;
    McpsIndication.Port = 0;
    McpsIndication.Multicast = 0;
    McpsIndication.FramePending = 0;
    McpsIndication.Buffer = NULL;
    McpsIndication.BufferSize = 0;
    McpsIndication.RxData = false;
    McpsIndication.AckReceived = false;
    McpsIndication.DownLinkCounter = 0;
    McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    LoRaMacFlags.Bits.SkipMcpsIndCb = 0;

    LORAMAC_TIMING_RX_DONE();

    Radio.Sleep( );
    TimerStop( &RxWindowTimer2 );

    macHdr.Value = payload[pktHeaderLen++];

    if( IsLoRaMacNetworkJoined == false )
    {
        /* Only JoinAccept is acceptable */
        macHdr.Bits.MType = FRAME_TYPE_JOIN_ACCEPT;
    }

    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( ( IsLoRaMacNetworkJoined == true ) || ( ( size != 17 ) && ( size != 33 ) )  )
            {
                goto JOIN_ABORT;
            }

            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            if( micRx == mic )
            {
                LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );

                // DLSettings
                Rx1DrOffset = ( LoRaMacRxPayload[11] >> 4 ) & 0x07;
                Rx2Channel.Datarate = LoRaMacRxPayload[11] & 0x0F;

                // RxDelay
                ReceiveDelay1 = ( LoRaMacRxPayload[12] & 0x0F );
                if( ReceiveDelay1 == 0 )
                {
                    ReceiveDelay1 = 1;
                }
                ReceiveDelay1 *= 1e3;
                ReceiveDelay2 = ReceiveDelay1 + 1e3;

                /* Not US915/US915HYBRID/AU920/CN470/CN470PREQUEL/STE920 */
                if( ( LORAMAC_STEPWIDTH_RX1_CHANNEL == 0 ) && ( LORAMAC_START_CH_ID != 8 ) )
                {
                    //CFList
                    if( ( size - 1 ) > 16 )
                    {
                        uint8_t i, imax;
                        ChannelParams_t param;
                        if( Channels[0].Frequency != 0 )
                        {
                            param = Channels[0];
                        }
                        else
                        {
                            param.DrRange.Value = ( DR_5 << 4 ) | DR_0;
                        }
                        if( param.DrRange.Fields.Min < LORAMAC_TX_MIN_DATARATE )
                        {
                            param.DrRange.Fields.Min = LORAMAC_TX_MIN_DATARATE;
                        }
                        if( param.DrRange.Fields.Max > LORAMAC_TX_MAX_DATARATE )
                        {
                            param.DrRange.Fields.Max = LORAMAC_TX_MAX_DATARATE;
                        }

                        i = LORAMAC_START_CH_ID;
                        imax = i + 5;

                        LoRaMacState |= MAC_TX_CONFIG;
                        for( uint8_t j = 0; i < imax; i++, j += 3 )
                        {
                            param.Frequency = ( ( uint32_t )LoRaMacRxPayload[13 + j] | ( ( uint32_t )LoRaMacRxPayload[14 + j] << 8 ) | ( ( uint32_t )LoRaMacRxPayload[15 + j] << 16 ) ) * 100;
                            if( param.Frequency != 0 )
                            {
                                LoRaMacChannelAdd( i, param );
                            }
                            else
                            {
                                LoRaMacChannelRemove( i );
                            }
                        }
                        LoRaMacState &= ~MAC_TX_CONFIG;
                    }
                }

                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                IsLoRaMacNetworkJoined = true;
                LoRaMacFlags.Bits.McpsInd = 1;
                JoinRequestTrials = 0;
                ChannelsDatarate = ChannelsDefaultDatarate;
                break;
            }
JOIN_ABORT:
            PrepareRxDoneAbort( );
            MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                address = payload[pktHeaderLen++];
                address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

                if( address != LoRaMacDevAddr )
                {
                    curMulticastParams = MulticastChannels;
                    while( curMulticastParams != NULL )
                    {
                        if( address == curMulticastParams->Address )
                        {
                            multicast = 1;
                            nwkSKey = curMulticastParams->NwkSKey;
                            appSKey = curMulticastParams->AppSKey;
                            downLinkCounter = curMulticastParams->DownLinkCounter;
                            break;
                        }
                        curMulticastParams = curMulticastParams->Next;
                    }
                    if( multicast == 0 )
                    {
                        // We are not the destination of this frame.
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                        PrepareRxDoneAbort( );
                        return;
                    }
                }
                else
                {
                    multicast = 0;
                    nwkSKey = LoRaMacNwkSKey;
                    appSKey = LoRaMacAppSKey;
                    downLinkCounter = DownLinkCounter;
                }

                fCtrl.Value = payload[pktHeaderLen++];

                sequenceCounter = ( uint16_t )payload[pktHeaderLen++];
                sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;

                micRx |= ( uint32_t )payload[size - LORAMAC_MFR_LEN];
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                sequenceCounterPrev = ( uint16_t )downLinkCounter;
                sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );

                if( sequenceCounterDiff < ( 1 << 15 ) )
                {
                    downLinkCounter += sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                }
                else
                {
                    // check for sequence roll-over
                    uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                        downLinkCounter = downLinkCounterTmp;
                    }
                }

                if( ( isMicOk == false ) && ( DownLinkCounterCheckingRelaxed == true ) )
                {
                    // enum check counter 0x0000xxxx to 0x007Fxxx ( 0 ~ 8388607 )
                    uint32_t i;
                    for(i=0; i<0x00800000;  i+=0x00010000){
                        downLinkCounter = sequenceCounter + i;
                        LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                        if( micRx == mic )
                        {
                            isMicOk = true;
                            break;
                        }
                    }
                }
                else if( DownLinkCounterCheckingRelaxed != true )
                {
                    // Check for a the maximum allowed counter difference
                    if( sequenceCounterDiff >= MAX_FCNT_GAP )
                    {
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS;
                        McpsIndication.DownLinkCounter = downLinkCounter;
                        PrepareRxDoneAbort( );
                        return;
                    }
                }

                if( isMicOk == true )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    McpsIndication.Multicast = multicast;
                    McpsIndication.FramePending = fCtrl.Bits.FPending;
                    McpsIndication.Buffer = NULL;
                    McpsIndication.BufferSize = 0;
                    McpsIndication.DownLinkCounter = downLinkCounter;

                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                    AdrAckCounter = 0;
                    MacCommandsBufferToRepeatIndex = 0;

                    // Update 32 bits downlink counter
                    if( multicast == 1 )
                    {
                        McpsIndication.McpsIndication = MCPS_MULTICAST;

                        if( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
                            ( curMulticastParams->DownLinkCounter != 0 ) )
                        {
                            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                            McpsIndication.DownLinkCounter = downLinkCounter;
                            PrepareRxDoneAbort( );
                            return;
                        }
                        curMulticastParams->DownLinkCounter = downLinkCounter;
                    }
                    else
                    {
                        if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                        {
                            SrvAckRequested = true;
                            McpsIndication.McpsIndication = MCPS_CONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                // Duplicated confirmed downlink. Skip indication.
                                //skipIndication = true;
                                LoRaMacFlags.Bits.SkipMcpsIndCb = 1;
                            }
                        }
                        else
                        {
                            SrvAckRequested = false;
                            McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                                McpsIndication.DownLinkCounter = downLinkCounter;
                                PrepareRxDoneAbort( );
                                return;
                            }
                        }
                        DownLinkCounter = downLinkCounter;
                    }

                    // Check if the frame is an acknowledgement
                    if( fCtrl.Bits.Ack == 1 )
                    {
                        McpsConfirm.AckReceived = true;
                        McpsIndication.AckReceived = true;

                        // Stop the AckTimeout timer as no more retransmissions
                        // are needed.
                        TimerStop( &AckTimeoutTimer );
                    }
                    else
                    {
                        McpsConfirm.AckReceived = false;

                        if( AckTimeoutRetriesCounter >= AckTimeoutRetries )
                        {
                            // Stop the AckTimeout timer as no more retransmissions
                            // are needed.
                            TimerStop( &AckTimeoutTimer );
                        }
                    }

                    if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( size - 4 ) - appPayloadStartIndex;

                        McpsIndication.Port = port;

                        if( port == 0 )
                        {
                            if( fCtrl.Bits.FOptsLen == 0 )
                            {
                                LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       nwkSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       LoRaMacRxPayload );

                                // Decode frame payload MAC commands
                                ProcessMacCommands( LoRaMacRxPayload, 0, frameLen, snr );
                            }
                            else
                            {
                                //skipIndication = true;
                                LoRaMacFlags.Bits.SkipMcpsIndCb = 1;
                            }
                        }
                        else
                        {
                            if( fCtrl.Bits.FOptsLen > 0 )
                            {
                                // Decode Options field MAC commands
                                ProcessMacCommands( payload, 8, appPayloadStartIndex - 1, snr );
                            }
                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   appSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   downLinkCounter,
                                                   LoRaMacRxPayload );

                            if( LoRaMacFlags.Bits.SkipMcpsIndCb == 0 )
                            {
                                McpsIndication.Buffer = LoRaMacRxPayload;
                                McpsIndication.BufferSize = frameLen;
                                McpsIndication.RxData = true;
                            }
                        }
                    }
                    else
                    {
                        if( fCtrl.Bits.FOptsLen > 0 )
                        {
                            // Decode Options field MAC commands
                            ProcessMacCommands( payload, 8, appPayloadStartIndex, snr );
                        }
                    }
//                    if( skipIndication == false )
//                    {
                        LoRaMacFlags.Bits.McpsInd = 1;
//                    }
                }
                else
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;
                    PrepareRxDoneAbort( );
                    return;
                }
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            {
                memcpy1( LoRaMacRxPayload, &payload[pktHeaderLen], size );

                McpsIndication.McpsIndication = MCPS_PROPRIETARY;
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                McpsIndication.Buffer = LoRaMacRxPayload;
                McpsIndication.BufferSize = size - pktHeaderLen;
                McpsIndication.RxData = true;

                LoRaMacFlags.Bits.McpsInd = 1;
                break;
            }
        default:
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }

    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );
    TimerStart( &MacStateCheckTimer );
}

static void OnRadioTxTimeout( void )
{
    LORAMAC_PIN_CLR();
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacFlags.Bits.MacDone = 1;
}

/* 1. RxSlot = 0, RXWIN1
   2. RxSlot = 1, RXWIN2
   3. RxSlot = 2, RXWIN2 Class C */
void OnRxException(LoRaMacEventInfoStatus_t Status)
{
    /* RxSlot will be overwrite by OnRxWindow2TimerEvent */
    uint8_t rxwin = RxSlot;

    LORAMAC_PIN_CLR();
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    /* 1. ( RxSlot == 1 ) this rxwin2 timeout occurs
       2. rxwin1 timeout and rxwin2 already passed */
    if( ( rxwin == 1 ) || ( (LoRaMacDeviceClass != CLASS_C) && ( !TimerExists( &RxWindowTimer2 ) ) ) )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = Status;
        }
        MlmeConfirm.Status = Status;
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void OnRadioRxError( void )
{
    uint8_t rxwin = RxSlot;
    OnRxException(LORAMAC_EVENT_INFO_STATUS_RX2_ERROR);
    if( rxwin == 0 )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;

        if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
        {
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
}

static void OnRadioRxTimeout( void )
{
    LORAMAC_TIMING_RX_DONE();
    OnRxException(LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT);
}

static void OnMacStateCheckTimerEvent( void )
{
    bool txTimeout = false;

    TimerStop( &MacStateCheckTimer );

    if( LoRaMacFlags.Bits.MacDone == 1 )
    {
        if( ( LoRaMacState & MAC_RX_ABORT ) == MAC_RX_ABORT )
        {
            LoRaMacState &= ~MAC_RX_ABORT;
            LoRaMacState &= ~MAC_TX_RUNNING;
        }

        if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
        {
            if( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )
            {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~MAC_TX_RUNNING;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                McpsConfirm.AckReceived = false;
                McpsConfirm.TxTimeOnAir = 0;
                txTimeout = true;
            }
        }

        if( ( NodeAckRequested == false ) && ( txTimeout == false ) )
        {
            if( LoRaMacFlags.Bits.MlmeReq == 1 )
            {
                if( MlmeConfirm.MlmeRequest == MLME_JOIN )
                {
                    // Retransmit only if the answer is not OK
                    //ChannelsNbRepCounter = 0;

                    //if( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_OK )
                    {
                        // Force stop retransmission
                        ChannelsNbRepCounter = ChannelsNbRep;
                        UpLinkCounter = 0;
                    }
                }
            }
            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
            {
                if( ( ChannelsNbRepCounter >= ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) )
                {
                    ChannelsNbRepCounter = 0;
                    AdrAckCounter++;
                    UpLinkCounter++;
                    LoRaMacState &= ~MAC_TX_RUNNING;
                }
                else
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                    // Sends the same frame again, unconfirmed message repetition
                    if( ScheduleTx( ) != LORAMAC_STATUS_OK )
                    {
                        LoRaMacFlags.Bits.MacDone = 1;

                        ChannelsNbRepCounter = 0;
                        AdrAckCounter++;
                        UpLinkCounter++;
                        LoRaMacState &= ~MAC_TX_RUNNING;
                    }
                }
            }
        }

        if( LoRaMacFlags.Bits.McpsInd == 1 )
        {
            //if( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            if(NodeAckRequested == true)
            {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                UpLinkCounter++;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

                LoRaMacState &= ~MAC_TX_RUNNING;
            }
        }

        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & MAC_TX_DELAYED ) == 0 ) )
        {
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;

                if( ( AdrCtrlOn == true ) && ( ( AckTimeoutRetriesCounter % 2 ) == 1 ) )
                {
                    ChannelsDatarate = MAX( ChannelsDatarate - 1, LORAMAC_TX_MIN_DATARATE );
                }
                LoRaMacFlags.Bits.MacDone = 0;
                // Sends the same frame again, confirmed message retry
                if( ScheduleTx( ) != LORAMAC_STATUS_OK )
                {
                    LoRaMacState &= ~MAC_TX_RUNNING;

                    LoRaMacFlags.Bits.MacDone = 1;

                    NodeAckRequested = false;
                    McpsConfirm.AckReceived = false;
                    McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                    UpLinkCounter++;
                }
            }
            else
            {
                LoRaMacExEnableDefaultChannels();

                LoRaMacState &= ~MAC_TX_RUNNING;

                NodeAckRequested = false;
                McpsConfirm.AckReceived = false;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                UpLinkCounter++;
            }
        }
    }
    // Handle reception for Class B and Class C
    if( ( LoRaMacState & MAC_RX ) == MAC_RX )
    {
        LoRaMacState &= ~MAC_RX;
    }
    if( LoRaMacState == MAC_IDLE )
    {
        if( LoRaMacFlags.Bits.McpsReq == 1 )
        {
            LoRaMacPrimitives->MacMcpsConfirm( &McpsConfirm );
            LoRaMacFlags.Bits.McpsReq = 0;
        }

        if( LoRaMacFlags.Bits.MlmeReq == 1 )
        {
            LoRaMacPrimitives->MacMlmeConfirm( &MlmeConfirm );
            LoRaMacFlags.Bits.MlmeReq = 0;
        }

        LoRaMacFlags.Bits.MacDone = 0;
    }
    else
    {
        // Operation not finished restart timer
        TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
        TimerStart( &MacStateCheckTimer );
    }

    if( LoRaMacFlags.Bits.McpsInd == 1 )
    {
        if( LoRaMacDeviceClass == CLASS_C )
        {// Activate RX2 window for Class C
            TimerStop( &AckTimeoutTimer );
            OnRxWindow2TimerEvent( );
        }
        if(LoRaMacFlags.Bits.SkipMcpsIndCb == 0){
            LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
        }
        LoRaMacFlags.Bits.SkipMcpsIndCb = 0;
        LoRaMacFlags.Bits.McpsInd = 0;
    }
}

static void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;

    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~MAC_TX_DELAYED;

    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_JOIN ) )
    {
        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of a join request retransmission, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    // Delayed TX
    if( ScheduleTx( ) != LORAMAC_STATUS_OK )
    {
        LoRaMacFlags.Bits.McpsReq = 1;
        LoRaMacFlags.Bits.MacDone = 1;
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    }
}

static void OnRxWindow1TimerEvent( void )
{
    int8_t datarate;
    uint32_t freq;

    TimerStop( &RxWindowTimer1 );
    RxSlot = 0;

    if( LoRaMacDeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

    datarate = LoRaMacExSetRx1DrOft(ChannelsDatarate);
    freq = LoRaMacExGetRx1Freq(Channel);

    RxWindowSetup( freq, datarate, DR_RFU, false );

    LORAMAC_TIMING_RX1_START();
}

static void OnRxWindow2TimerEvent( void )
{
    bool continuos;

    TimerStop( &RxWindowTimer2 );

    if( Radio.GetStatus( ) != RF_IDLE )
    {
        return;
    }

    RxSlot = 1;

    if( LoRaMacDeviceClass == CLASS_C )
    {
        RxSlot = 2;
        continuos = true;
    }
    else
    {
        continuos = false;
    }

    RxWindowSetup( Rx2Channel.Frequency, Rx2Channel.Datarate, Rx2Channel.DrValue, continuos );

    LORAMAC_TIMING_RX2_START();
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &AckTimeoutTimer );

    if( NodeAckRequested == true )
    {
        AckTimeoutRetry = true;
        LoRaMacState &= ~MAC_ACK_REQ;
    }
    if( LoRaMacDeviceClass == CLASS_C )
    {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

void RemoveMemberOfArray(uint8_t *buf, uint8_t len, uint8_t index)
{
    memcpy1(buf+index, buf+index+1, len-1-index);
}

TimerTime_t RegionCommonUpdateBandTimeOff( void )
{
    /* set to maximum value */
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );

    // Update bands Time OFF

    for( uint8_t i = 0; i < LORA_MAX_NB_BANDS; i++ )
    {
        if( ( ( IsLoRaMacNetworkJoined == false ) && ( LoRaMacJoinDutyCycle == true ) ) || ( DutyCycleOn == true ) )
        {
            if( Bands[i].TimeOff > TimerGetElapsedTime( Bands[i].LastTxDoneTime ) )
            {
                nextTxDelay = MIN( Bands[i].TimeOff - TimerGetElapsedTime( Bands[i].LastTxDoneTime ), nextTxDelay );
            }
            else
            {
                Bands[i].TimeOff = 0;
            }
        }
        else
        {
            Bands[i].TimeOff = 0;
            nextTxDelay = 0;
        }
    }

    if( nextTxDelay == ( TimerTime_t )( -1 ) )
    {
        nextTxDelay = 0;
    }
    return nextTxDelay;
}

uint8_t CountNbOfEnabledChannels( uint16_t* chmsk, uint8_t* enabledChannels, uint8_t* delayTx )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTransmission = 0;

    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS; i++)
    {
        if( chmsk[i/16] & (1<<(i%16)) )
        {
            if( Channels[i].Frequency == 0 )
            { // Check if the channel is enabled
                continue;
            }
            if( IsLoRaMacNetworkJoined == false )
            {
                if( ( (0xFFFF >> (16-LORAMAC_START_CH_ID)) & ( 1 << (i%16) ) ) == 0 )
                {
                    continue;
                }
            }
            if( ( ( Channels[i].DrRange.Fields.Min <= ChannelsDatarate ) &&
                 ( ChannelsDatarate <= Channels[i].DrRange.Fields.Max ) ) == false )
            { // Check if the current channel selection supports the given datarate
                continue;
            }
            if( Bands[Channels[i].Band].TimeOff > 0 )
            { // Check if the band is available for transmission
                delayTransmission++;
                continue;
            }
            if( enabledChannels != NULL )
            {
                enabledChannels[nbEnabledChannels++] = i;
            }
        }
    }

    *delayTx = delayTransmission;

    return nbEnabledChannels;
}

static LoRaMacStatus_t SetNextChannel( TimerTime_t* time )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTx = 0;
    uint8_t enabledChannels[LORA_MAX_CHS];
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );
    uint16_t *chmsk;
    uint16_t chmsksum = 0;
    uint8_t ch_remain_max;
    int i;

    memset1( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

    ch_remain_max = LORA_MAX_NB_CHANNELS;

    for(i=0; i<6; i++)
    {
        ChannelsMaskRemaining[i] &= ChannelsMask[i];
    }

    /* US915/US915HYBRID/AU915/AU915OLD */
    if(LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000)
    {
        ch_remain_max = 64;
        if( CountNbEnabled125kHzChannels( ChannelsMaskRemaining ) == 0 )
        { // Restore default channels
            memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) ChannelsMask, 8 );
        }
        if( ( ChannelsDatarate >= LORAMAC_TX_MAX_DATARATE ) && ( ( ChannelsMaskRemaining[4] & 0x00FF ) == 0 ) )
        { // Make sure, that the channels 64 ~ 71 are activated
            ChannelsMaskRemaining[4] = ChannelsMask[4];
        }
        chmsk = ChannelsMaskRemaining;
    }
    else
    {
        /* validate channel mask, reset if no channel is enabled */
        chmsk = ChannelsMask;
        for( i=0; i<6; i++ )
        {
            chmsksum += ChannelsMask[i];
        }
        if( chmsksum == 0 )
        {
            LoRaMacExEnableDefaultChannels();
        }
    }

    // Update Aggregated duty cycle
    if( AggregatedTimeOff <= TimerGetElapsedTime( AggregatedLastTxDoneTime ) )
    {
        AggregatedTimeOff = 0;
        nextTxDelay = RegionCommonUpdateBandTimeOff();
        nbEnabledChannels = CountNbOfEnabledChannels(chmsk, enabledChannels, &delayTx);
    }
    else
    {
        delayTx++;
        nextTxDelay = AggregatedTimeOff - TimerGetElapsedTime( AggregatedLastTxDoneTime );
    }

    if( nbEnabledChannels > 0 )
    {
        uint8_t index;
        while(nbEnabledChannels > 0)
        {
            index = randr( 0, nbEnabledChannels - 1 );
            if( Radio.IsChannelFree( MODEM_LORA, Channels[enabledChannels[index]].Frequency, LORAMAC_RSSI_THRESH ) == true )
            {
                if( (PhyType == AS923) || (PhyType == KR920) )
                {
                    DelayMs(6);
                    if( Radio.IsChannelFree( MODEM_LORA, Channels[enabledChannels[index]].Frequency, LORAMAC_RSSI_THRESH ) == true )
                    {
                        Channel = enabledChannels[index];
                        break;
                    }
                }
                else
                {
                    Channel = enabledChannels[index];
                    break;
                }
            }
            else
            {
                 memcpy1(enabledChannels+index, enabledChannels+index+1, nbEnabledChannels-1-index);
                 nbEnabledChannels--;
            }
        }
        if(nbEnabledChannels == 0)
        {
            return LORAMAC_STATUS_NO_FREE_CHANNEL;
        }

        if( Channel < ch_remain_max )
        {
            DisableChannelInMask( Channel, ChannelsMaskRemaining );
        }

        *time = 0;
        return LORAMAC_STATUS_OK;
    }
    else
    {
        if( delayTx > 0 )
        {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = nextTxDelay;
            return LORAMAC_STATUS_OK;
        }
        // Datarate not supported by any channel
        *time = 0;
        return LORAMAC_STATUS_DATARATE_NOT_SUPPORTED;
    }
}

void SetPublicNetwork( bool enable )
{
    PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if( PublicNetwork == true )
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

static void RxWindowSetup( uint32_t freq, int8_t datarate, uint8_t drval, bool rxContinuous )
{
    uint8_t sf;
    uint8_t dr_tab;
    uint8_t bw;
    RadioModems_t modem;
    uint16_t timeout;

    if(drval != DR_RFU){
        dr_tab = drval;
        datarate = LORAMAC_RX_MAX_DATARATE;
    }else{
        dr_tab = LoRaMacDrTab[datarate];
    }

    /** If DR is in valid, set to EU868 DR5 SF7/BW125KHz */
    if(dr_tab == DR_RFU){
        dr_tab =LoRaMacEu868DrTab[DR_5];
    }

    sf = dr_tab & 0x0F;
    bw = (dr_tab>>4) & 0x03;

    timeout = LoRaMacExGetRxWinSymbols(sf, bw);

    if( Radio.GetStatus( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );
        if( sf == 0 )
        {
            Radio.SetRxConfig( MODEM_FSK, 50000, 50000, 0, 83333, 5, timeout, false, 0, true, 0, 0, false, rxContinuous );
            modem = MODEM_FSK;
        }
        else
        {
            Radio.SetRxConfig( MODEM_LORA, bw, sf, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
            modem = MODEM_LORA;
        }

        Radio.SetMaxPayloadLength( modem, DlMaxPayloadOfDatarate[datarate] + LORA_MAC_FRMPAYLOAD_OVERHEAD );

        LORAMAC_PIN_SET();
        if( rxContinuous == false )
        {
            Radio.Rx( MaxRxWindow );
        }
        else
        {
            Radio.Rx( 0 ); // Continuous mode
        }
    }
}

//static bool Rx2FreqInRange( uint32_t freq )
//{
//#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
//    if( Radio.CheckRfFrequency( freq ) == true )
//#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
//    if( ( Radio.CheckRfFrequency( freq ) == true ) &&
//        ( freq >= LORAMAC_FIRST_RX2_CHANNEL ) &&
//        ( freq <= LORAMAC_LAST_RX2_CHANNEL ) &&
//        ( ( ( freq - ( uint32_t ) LORAMAC_FIRST_RX2_CHANNEL ) % ( uint32_t ) LORAMAC_STEPWIDTH_RX2_CHANNEL ) == 0 ) )
//#endif
//    {
//        return true;
//    }
//    switch(phytype){
//    case EU433:	    // 433 ~ 434
//    case EU868:     // 863 ~ 870
//    case CN780:		// 779 ~ 787
//        return Radio.CheckRfFrequency( freq );
//    case CUSTOM:    // full
//    case CN470:	    // 470 ~ 510
//    default:
//        return true;
//    case US915:		// 902 ~ 928
//    case AU920:		// 915 ~ 928
//    case US915HYBRID:
//
//        break;
//    }
//}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Get the maximum payload length
    maxN = MaxPayloadOfDatarate[datarate];

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Port field takes one byte
    if(lenN > 0){
        payloadSize++;
    }

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
}

uint8_t CountChannelMask(uint16_t *channelsMask, uint8_t channelsMaskNum)
{
    uint8_t nbActiveBits = 0;

    for( uint8_t i = 0; i < channelsMaskNum*16; i++ )
    {
        if( ( channelsMask[i/16] & ( 1 << ( i%16 ) ) ) != 0 )
        {
            nbActiveBits++;
        }
    }

    return nbActiveBits;
}

static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask )
{
    return CountChannelMask(channelsMask, 4);
}

//static void ReenableChannels( uint16_t mask, uint16_t* channelMask )
//{
//    uint16_t blockMask = mask;
//
//    for( uint8_t i = 0, j = 0; i < 4; i++, j += 2 )
//    {
//        channelMask[i] = 0;
//        if( ( blockMask & ( 1 << j ) ) != 0 )
//        {
//            channelMask[i] |= 0x00FF;
//        }
//        if( ( blockMask & ( 1 << ( j + 1 ) ) ) != 0 )
//        {
//            channelMask[i] |= 0xFF00;
//        }
//    }
//    channelMask[4] = blockMask;
//    channelMask[5] = 0x0000;
//}

/* Make sure 125KHz channel is enabled according to the 500KHz channels (vice versa) */
bool ValidateChannelsMask( uint16_t* channelMask )
{
    uint8_t i;
    uint8_t index;
    uint16_t mask;
    uint16_t chMask;
    uint16_t chMask500;
    bool chanMaskState = false;

    channelMask[4] &= 0x00FF;
    channelMask[5] = 0x0000;
    chMask500 = channelMask[4];

    for(i=0; i<8; i++)
    {
        /*
        i -> ch500 ch125   maskPos maskIndex
        0 -> 64,  0 - 7     low    0
        1 -> 65,  8 - 15   high    0
        2 -> 66, 16 - 23    low    1
        3 -> 67, 24 - 31   high    1
        ...
        6 -> 70, 64 - 71     low   3
        7 -> 71, 56 - 63    high   3
        */
        index = i/2;
        mask = ( 0xFF << ( (i%2) * 8 ) );

        /* save and clear channel mask */
        chMask = channelMask[index] & mask;
        channelMask[index] &= ~mask;

        if( chMask500 != 0 )
        {
            if( chMask500 & ( 1 << i ) )
            {
                chanMaskState = true;
                if( CountChannelMask( &chMask, 1 ) > 5 )
                {
                    channelMask[index] |= chMask;
                }
                else
                {
                    channelMask[index] |= mask;
                }
            }
        }
        else
        {
            if( CountChannelMask( &chMask, 1 ) > 5 )
            {
                channelMask[4] |= (1<<i);
                channelMask[index] |= chMask;
                chanMaskState = true;
            }
        }
    }
    return chanMaskState;
}

static int8_t LimitTxPower( int8_t txPower, int8_t maxBandTxPower )
{
    int8_t resultTxPower = txPower;
    if(LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000)
    {
        if( ChannelsDatarate == LORAMAC_TX_MAX_DATARATE  )
        {// 500KHz channel, Limit tx power to max 26dBm
            resultTxPower =  MAX( txPower, US915_TX_POWER_26_DBM );
        }
        else
        {
            if( CountNbEnabled125kHzChannels( ChannelsMask ) < 50 )
            {// Limit tx power to max 21dBm
                resultTxPower = MAX( txPower, US915_TX_POWER_20_DBM );
            }
        }
    }
    else
    {
        /* Power index the larger, output power the smaller */
        resultTxPower =  MAX( txPower, maxBandTxPower );
    }
    return resultTxPower;
}

static bool ValueInRange( int8_t value, int8_t min, int8_t max )
{
    if( ( value >= min ) && ( value <= max ) )
    {
        return true;
    }
    return false;
}

static bool DisableChannelInMask( uint8_t id, uint16_t* mask )
{
    // Deactivate channel
    mask[id / 16] &= ~( 1 << ( id % 16 ) );
    return true;
}

static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut )
{
    bool adrAckReq = false;
    int8_t datarate = ChannelsDatarate;

    if( adrEnabled == true )
    {
        if( datarate == LORAMAC_TX_MIN_DATARATE )
        {
            AdrAckCounter = 0;
            adrAckReq = false;
        }
        else
        {
            if( AdrAckCounter >= ADR_ACK_LIMIT )
            {
                adrAckReq = true;
                ChannelsTxPower = LORAMAC_MAX_TX_POWER;
            }
            else
            {
                adrAckReq = false;
            }
            if( AdrAckCounter >= ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
            {
                if( ( AdrAckCounter % ADR_ACK_DELAY ) == 1 )
                {
                    /* make sure datarate is not out of range */
                    if( datarate > LORAMAC_TX_MAX_DATARATE )
                    {
                        datarate = LORAMAC_TX_MAX_DATARATE;
                    }
                    if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
                            // Re-enable default channels
                            LoRaMacExEnableDefaultChannels();
                        }
                    }
                }
            }
        }
    }

    *datarateOut = datarate;

    return adrAckReq;
}

static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_BUSY;
    // The maximum buffer length must take MAC commands to re-send into account.
    uint8_t bufLen = LORA_MAC_COMMAND_MAX_LENGTH - MacCommandsBufferToRepeatIndex;

    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 2 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCommandsBuffer[MacCommandsBufferIndex++] = (p2>>2);
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_TX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: MaxEIRP, UplinkDwellTime and DownlinkDwellTime
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DL_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: ChannelFrequency and UplinkFrequency
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }
    if( status == LORAMAC_STATUS_OK )
    {
        MacCommandsInNextTx = true;
    }
    return status;
}

static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut )
{
    uint8_t i = 0;
    uint8_t cmdCount = 0;

    for( i = 0; i < length; i++ )
    {
        switch( cmdBufIn[i] )
        {
            // STICKY
            case MOTE_MAC_RX_PARAM_SETUP_ANS:
            case MOTE_MAC_DL_CHANNEL_ANS:
            { // 1 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            case MOTE_MAC_RX_TIMING_SETUP_ANS:
            { // 0 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            // NON-STICKY
            case MOTE_MAC_DEV_STATUS_ANS:
            { // 2 bytes payload
                i += 2;
                break;
            }
            case MOTE_MAC_LINK_ADR_ANS:
            case MOTE_MAC_NEW_CHANNEL_ANS:
            { // 1 byte payload
                i++;
                break;
            }
            case MOTE_MAC_DUTY_CYCLE_ANS:
            case MOTE_MAC_LINK_CHECK_REQ:
            case MOTE_MAC_TX_PARAM_SETUP_ANS:
            { // 0 byte payload
                break;
            }
            default:
                break;
        }
    }

    return cmdCount;
}

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr )
{
    if( LoRaMacExCheckSrvMacCommand(payload+macIndex, commandsSize - macIndex) == false ){
        /* Server mac command is invalid */
#ifdef USE_DEBUGGER
        //printf("MAC CMD ERROR\n");
#endif
        return;
    }

    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                MlmeConfirm.DemodMargin = payload[macIndex++];
                MlmeConfirm.NbGateways = payload[macIndex++];
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint16_t chMaskBitmap;
                    uint8_t chMaskIndex;
                    uint8_t status = 0x07;
                    uint16_t chMask;
                    int8_t txPower;
                    int8_t datarate;
                    uint8_t nbRep;
                    uint8_t chMaskCntl;
                    uint16_t channelsMask[6];
                    bool chmask_fail = false;
                    bool power_fail = false;
                    bool datarate_fail = false;
                    uint16_t drmap;
                    uint8_t chNumMax;
                    uint8_t chNumStart;

                    // Initialize local copy of the channels mask array
                    for( i = 0; i < 6; i++ )
                    {
                        channelsMask[i] = ChannelsMask[i];
                    }

                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    chMask = ( uint16_t )payload[macIndex++];
                    chMask |= ( uint16_t )payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }

                    chmask_fail = false;

                    if( ( AdrCtrlOn == false ) && ( ( ChannelsDatarate != datarate ) || ( ChannelsTxPower != txPower ) ) )
                    {
                        // ADR disabled don't handle ADR requests if server tries to change datarate or txpower
                        // Answer the server with fail status
                        // Power ACK     = 0
                        // Data rate ACK = 0
                        // Channel mask  = 0
                        AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                        break;
                    }

                    // WARNING: Channel mask will be ignored in custom data rate mode
                    if(PhyType == CN470)
                    {
                        if( chMaskCntl == 7 )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
                        else if( chMaskCntl == 6 )
                        {
                            chNumStart = 0;
                            chNumMax = LORA_MAX_NB_CHANNELS;
                        }else{
                            chNumStart = chMaskCntl*16;
                            chNumMax = chNumStart + 16;
                        }
                    }
                    else if( LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000 )
                    {
                        if( chMaskCntl == 7 )
                        {
                            // Disable all 125 kHz channels
                            channelsMask[0] = 0x0000;
                            channelsMask[1] = 0x0000;
                            channelsMask[2] = 0x0000;
                            channelsMask[3] = 0x0000;
                            channelsMask[4] = chMask;
                        }
                        else if( chMaskCntl == 6 )
                        {
                            chNumMax = 64;
                            chNumStart = 0;
                            channelsMask[4] = chMask;
                        }
                        else if( chMaskCntl == 5 )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
                        else
                        {
                            chNumStart = chMaskCntl*16;
                            chNumMax = chNumStart + 16;
                        }
                    }
                    else
                    {
                        switch(chMaskCntl)
                        {
                        case 6:
                        case 0:
                            chNumStart = 0;
                            chNumMax = 16;
                            break;
                        default:
                            status &= 0xFE; // Channel mask KO
                            break;
                        }
                    }

                    if( status == 0x07 )
                    {
                        for( i = chNumStart; i < chNumMax; i++ )
                        {
                            chMaskIndex = i/16;
                            chMaskBitmap = 1 << (i%16);
                            if( chMaskCntl == 6 )
                            {
                                if( Channels[i].Frequency != 0 )
                                {
                                    channelsMask[chMaskIndex] |= chMaskBitmap;
                                }
                            }
                            else
                            {
                                // Trying to enable an undefined channel
                                if( ( ( chMask & chMaskBitmap ) != 0 ) && ( Channels[i].Frequency == 0 ) )
                                {
                                    chMask &= ~chMaskBitmap;
                                    chmask_fail = true;
                                }
                            }
                        }
                        if( chMaskCntl != 6 )
                        {
                            channelsMask[chMaskIndex] = chMask;
                        }
                    }

                    if( LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000 )
                    {
                        //FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
                        if( ( datarate < LORAMAC_TX_MAX_DATARATE ) && ( CountNbEnabled125kHzChannels( channelsMask ) < 2 ) )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
                        if( PhyType == US915HYBRID )
                        {
                            if( ValidateChannelsMask( channelsMask ) == false )
                            {
                                status &= 0xFE; // Channel mask KO
                            }
                        }
                    }

                    if( CountChannelMask(channelsMask, 6) == 0 )
                    {
                        status &= 0xFE;
                    }

                    if( ValueInRange( datarate, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // TxPower KO
                    }
                    else
                    {
                        drmap = ChannelDrMap(channelsMask);
                        if( ( drmap & (1<<datarate) ) == 0 )
                        {
                            /* No available data rate found, choose best DR automactically */
                            datarate_fail = true;
                            datarate = DataRateFormat(drmap, datarate);
                        }
                    }

                    /* LORAMAC_MAX_TX_POWER numbermic value is less than LORAMAC_MIN_TX_POWER */
                    if( ValueInRange( txPower, LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    else
                    {
                        /* US915 */
                        if( TxPowers[txPower] > SX1276SetRfTxPower( TxPowers[txPower], Channels[0].Frequency, false ) )
                        {
                            power_fail = true;
                        }
                    }
#ifdef ENABLE_LWCT
                    /* Check ChannelMask and Power strictly */
                    if( ( ( status & 0x07 ) == 0x07 ) && ( chmask_fail == false ) && ( power_fail == false ) && (datarate_fail == false) )
#else
                    if( ( status & 0x07 ) == 0x07 )
#endif
                    {
                        ChannelsDatarate = datarate;
                        ChannelsTxPower = txPower;
                        memcpy1((uint8_t *)ChannelsMask, (uint8_t *)channelsMask, 12);
                        ChannelsNbRep = nbRep;
                        if( LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000 )
                        {
                            // Reset ChannelsMaskRemaining to the new ChannelsMask
                            ChannelsMaskRemaining[0] &= channelsMask[0];
                            ChannelsMaskRemaining[1] &= channelsMask[1];
                            ChannelsMaskRemaining[2] &= channelsMask[2];
                            ChannelsMaskRemaining[3] &= channelsMask[3];
                            ChannelsMaskRemaining[4] = channelsMask[4];
                            ChannelsMaskRemaining[5] = channelsMask[5];
                        }
                    }
                    if(chmask_fail)
                    {
                        status &= 0xFE; // ChMask KO
                    }
                    if(datarate_fail)
                    {
                        status &= 0xFD; // Datarate KO
                    }
                    if(power_fail)
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;

                    drOffset = ( payload[macIndex] >> 4 ) & 0x07;
                    datarate = payload[macIndex] & 0x0F;
                    macIndex++;

                    freq =  ( uint32_t )payload[macIndex++];
                    freq |= ( uint32_t )payload[macIndex++] << 8;
                    freq |= ( uint32_t )payload[macIndex++] << 16;
                    freq *= 100;

                    if( LoRaMacExRx2FreqInRange( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ValueInRange( datarate, LORAMAC_RX_MIN_DATARATE, LORAMAC_RX_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    if( ValueInRange( drOffset, LORAMAC_MIN_RX1_DR_OFFSET, LORAMAC_MAX_RX1_DR_OFFSET ) == false )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        Rx2Channel.Datarate = datarate;
                        Rx2Channel.Frequency = freq;
                        Rx2Channel.DrValue = DR_RFU;
                        Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                {
                    uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetBatteryLevel != NULL ) )
                    {
                        batteryLevel = LoRaMacCallbacks->GetBatteryLevel( );
                    }
                    AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr );
                    break;
                }
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t channelIndex = 0;
                    int8_t channelIndexMin = 3;
                    ChannelParams_t chParam;

                    channelIndex = payload[macIndex++];
                    chParam.Frequency = ( uint32_t )payload[macIndex++];
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];

                    if( LORAMAC_STEPWIDTH_RX1_CHANNEL != 0 )
                    {
                        status &= 0xFC; // Channel frequency and datarate KO
                    }
                    else
                    {
                        LoRaMacState |= MAC_TX_CONFIG;

                        if( PhyType == AS923 )
                        {
                            channelIndexMin = 2;
                        }

                        if( channelIndex < channelIndexMin )
                        {
                            status &= 0xFC;
                        }
                        else
                        {
                            if( chParam.Frequency == 0 )
                            {
                                if( LoRaMacChannelRemove( channelIndex ) != LORAMAC_STATUS_OK )
                                {
                                    status &= 0xFC;
                                }
                            }
                            else
                            {
                                switch( LoRaMacChannelAdd( channelIndex, chParam ) )
                                {
                                    case LORAMAC_STATUS_OK:
                                    {
                                        break;
                                    }
                                    case LORAMAC_STATUS_FREQUENCY_INVALID:
                                    {
                                        status &= 0xFE;
                                        break;
                                    }
                                    case LORAMAC_STATUS_DATARATE_INVALID:
                                    {
                                        status &= 0xFD;
                                        break;
                                    }
                                    case LORAMAC_STATUS_FREQ_AND_DR_INVALID:
                                    {
                                        status &= 0xFC;
                                        break;
                                    }
                                    default:
                                    {
                                        status &= 0xFC;
                                        break;
                                    }
                                }
                            }
                        }
                        LoRaMacState &= ~MAC_TX_CONFIG;
                    }
                    AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;

                    if( delay == 0 )
                    {
                        delay++;
                    }
                    ReceiveDelay1 = delay * 1e3;
                    ReceiveDelay2 = ReceiveDelay1 + 1e3;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
             case SRV_MAC_TX_PARAM_SETUP_REQ:
                {
                    struct
                    {
                        uint8_t MaxEIRP             : 4;
                        uint8_t UplinkDwellTime     : 1;
                        uint8_t DownlinkDwellTime   : 1;
                        uint8_t RFU                 : 2;
                    }EirpDwellTime;

                    *(uint8_t *)&EirpDwellTime = payload[macIndex++];

                    /* Only AS923 supports this command */
                    if( PhyType != AS923 )
                    {
                        break;
                    }

                    if( 0 == LoRaMacExSetMaxEirp( EirpDwellTime.MaxEIRP ) )
                    {
                        LoRaMacExSetDwellTime(EirpDwellTime.UplinkDwellTime, EirpDwellTime.DownlinkDwellTime);
                        AddMacCommand( MOTE_MAC_TX_PARAM_SETUP_ANS, 0, 0 );
                    }
                }
                break;
            case SRV_MAC_DL_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    int8_t channelIndex = 0;
                    uint32_t frequency;

                    channelIndex = payload[macIndex++];
                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;

                    if(channelIndex >= LORA_MAX_CHS){
                        status &= 0x02;
                    }

                    if(frequency != 0){
                        if( LoRaMacExCheckFreq(frequency) < 0 ){
                            status &= 0x01;
                        }
                    }

                    if(status == 0x03){
                        DlChannel[channelIndex] = frequency;
                    }

                    AddMacCommand( MOTE_MAC_DL_CHANNEL_ANS, status, 0 );
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

    fCtrl.Value = 0;
    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );

    // Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

    // Reset confirm parameters
    McpsConfirm.NbRetries = 0;
    McpsConfirm.AckReceived = false;
    McpsConfirm.UpLinkCounter = UpLinkCounter;

    status = ScheduleTx( );

    return status;
}

uint16_t ChannelDrMap(uint16_t *chmsk)
{
    uint8_t i, j;
    uint16_t drmap;

    drmap = 0;

    for( i = 0; i < LORA_MAX_NB_CHANNELS; i++)
    {
        if( chmsk[i/16] & (1<<(i%16)) )
        {
            if( Channels[i].Frequency == 0 )
            { // Check if the channel is enabled
                continue;
            }

            for(j=Channels[i].DrRange.Fields.Min; j<=Channels[i].DrRange.Fields.Max; j++)
            {
                drmap |= 1<<j;
            }
        }
    }

    return drmap;
}

uint8_t DataRateFormat(uint16_t drmap, uint8_t datarate)
{
    while(datarate > LORAMAC_TX_MIN_DATARATE)
    {
        if( ( drmap & (1<<datarate) ) != 0 )
        {
            break;
        }
        datarate--;
    }
    if( ( drmap & (1<<datarate) ) == 0 )
    {
        while(datarate < LORAMAC_TX_MAX_DATARATE)
        {
            if( ( drmap & (1<<datarate) ) != 0 )
            {
                break;
            }
            datarate++;
        }
    }
    return datarate;
}

static LoRaMacStatus_t ScheduleTx( )
{
    TimerTime_t dutyCycleTimeOff = 0;
    LoRaMacStatus_t status;
    int drError = 0;
    uint8_t dr_val;
    int8_t rxwin_time_oft1, rxwin_time_oft2;
    uint16_t drmap;

    // Check if the device is off
    if( MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }

    /* Paylaod is ready to sent now, need check the whole payload size */

    if( ( LoRaMacBufferPktLen - LORA_MAC_FRMPAYLOAD_OVERHEAD ) > MaxPayloadOfDatarate[ChannelsDatarate] )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    // Select channel
    while( 1 )
    {
        status = SetNextChannel( &dutyCycleTimeOff );
        if(status == LORAMAC_STATUS_DATARATE_NOT_SUPPORTED)
        {
            drError++;
            if(drError > 3)
            {
                return status;
            }
        }
        else if(status == LORAMAC_STATUS_NO_FREE_CHANNEL)
        {
            return status;
        }
        else
        {
            break;
        }

        drmap = ChannelDrMap(ChannelsMask);
        if( 0 == drmap ){
            // Set the default datarate
            ChannelsDatarate = ChannelsDefaultDatarate;
            LoRaMacExEnableDefaultChannels();
        }else{
            ChannelsDatarate = DataRateFormat(drmap, ChannelsDatarate);
        }
        LoRaMacExEnableDefaultChannels();
    }

    if(Rx2Channel.DrValue == DR_RFU){
        dr_val = LoRaMacDrTab[Rx2Channel.Datarate];
    }else{
        dr_val = Rx2Channel.DrValue;
    }
    rxwin_time_oft1 = LoRaMacExGetRxWinTimeOffset(LoRaMacDrTab[LoRaMacExSetRx1DrOft(ChannelsDatarate)]);
    rxwin_time_oft2 = LoRaMacExGetRxWinTimeOffset(dr_val);

    if( IsLoRaMacNetworkJoined == false )
    {
        RxWindow1Delay = JoinAcceptDelay1 + rxwin_time_oft1 - RADIO_WAKEUP_TIME;
        RxWindow2Delay = JoinAcceptDelay2 + rxwin_time_oft2 - RADIO_WAKEUP_TIME;
    }
    else
    {
        RxWindow1Delay = ReceiveDelay1 + rxwin_time_oft1 - RADIO_WAKEUP_TIME;
        RxWindow2Delay = ReceiveDelay2 + rxwin_time_oft2 - RADIO_WAKEUP_TIME;

        if( ( LoRaMacBufferPktLen - LORA_MAC_FRMPAYLOAD_OVERHEAD ) > MaxPayloadOfDatarate[ChannelsDatarate] )
        {
            /* SetNextChannel could breaks Radio RX status, it is neccessary to restore Class C RX status on failure */
            if( LoRaMacDeviceClass == CLASS_C )
            {// Activate RX2 window for Class C
                OnRxWindow2TimerEvent( );
            }
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
    }

    // Schedule transmission of frame
    if( dutyCycleTimeOff == 0 )
    {
        // Try to send now
        return SendFrameOnChannel( Channels[Channel] );
    }
    else
    {
        // Send later - prepare timer
        LoRaMacState |= MAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, dutyCycleTimeOff );
        TimerStart( &TxDelayedTimer );

        return LORAMAC_STATUS_OK;
    }
}

static uint16_t JoinDutyCycle( void )
{
    uint16_t dutyCycle = 0;
    /* Clear every 24 hour */
    TimerTime_t timeElapsed = TimerGetElapsedTime( LoRaMacInitializationTime );

    if( LoRaMacJoinDutyCycle || DutyCycleOn )
    {
        if( timeElapsed < 3600000 )
        {
            /* First one hour */
            dutyCycle = BACKOFF_DC_1_HOUR;
        }
        else if( timeElapsed < ( 3600000 + 36000000 ) )
        {
            /* Next 10 hours */
            dutyCycle = BACKOFF_DC_10_HOURS;
        }
        else
        {
            /* Next N * 24 hours */
            dutyCycle = BACKOFF_DC_24_HOURS;
        }
    }

    return dutyCycle;
}

static void CalculateBackOff( uint8_t channel )
{
    uint16_t dutyCycle = Bands[Channels[channel].Band].DCycle;
    uint16_t joinDutyCycle = 0;

    // Reset time-off to initial value.
    Bands[Channels[channel].Band].TimeOff = 0;

    if( IsLoRaMacNetworkJoined == false )
    {
        // The node has not joined yet. Apply join duty cycle to all regions.
        joinDutyCycle = JoinDutyCycle( );
        dutyCycle = MAX( dutyCycle, joinDutyCycle );

        // Update Band time-off.
        Bands[Channels[channel].Band].TimeOff = TxTimeOnAir * dutyCycle - TxTimeOnAir;
    }
    else
    {
        if( DutyCycleOn == true )
        {
            Bands[Channels[channel].Band].TimeOff = TxTimeOnAir * dutyCycle - TxTimeOnAir;
        }
    }

    // Update Aggregated Time OFF
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void* payload = fBuffer;
    uint8_t framePort = fPort;
    LoRaMacStatus_t status;

    LoRaMacBufferPktLen = 0;

    NodeAckRequested = false;

    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    if( fBufferSize == 0 )
    {
        payload = NULL;
    }

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:
            LoRaMacBufferPktLen = pktHeaderLen;

            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacAppEui, 8 );
            LoRaMacBufferPktLen += 8;
            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacDevEui, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = Radio.Random( );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
            //Intentional falltrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            fCtrl->Bits.AdrAckReq = AdrNextDr( fCtrl->Bits.Adr, true, &ChannelsDatarate );

            status = LoRaMacQueryTxPossible( fBufferSize, NULL );
            if( status != LORAMAC_STATUS_OK )
            {
                return status;
            }

            LoRaMacExUpdateMacCommandMaxLen(ChannelsDatarate);

            if( SrvAckRequested == true )
            {
                SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }

            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            if( MacCommandsInNextTx == true )
            {
                bool macCommandSentFlag;

                // Copy the MAC commands which must be re-send into the MAC command buffer
                memcpy1( &MacCommandsBuffer[MacCommandsBufferIndex], MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
                MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

                macCommandSentFlag = false;

                if( ( MacCommandsBufferIndex > LORA_MAC_COMMAND_FOPTS_MAX_LENGTH ) || ( ( framePort == 0 ) && ( fBufferSize == 0 ) ) )
                {
                    // force to use port 0
                    payload = MacCommandsBuffer;
                    fBufferSize = MacCommandsBufferIndex;
                    framePort = 0;
                    macCommandSentFlag = true;
                }
                else if( MacCommandsBufferIndex <= LORA_MAC_COMMAND_FOPTS_MAX_LENGTH )
                {
                    fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                    // Update FCtrl field with new value of OptionsLength
                    LoRaMacBuffer[0x05] = fCtrl->Value;
                    for( i = 0; i < MacCommandsBufferIndex; i++ )
                    {
                        LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                    }

                    /* Port 0 Mac command and Fopts command can't be send together, skip appliaction layer mac command */
                    if( ( framePort == 0 ) && ( fBufferSize != 0 ) )
                    {
                        fBufferSize = 0;
                    }

                    macCommandSentFlag = true;
                }

                if( macCommandSentFlag == true )
                {
                    // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
                    MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex, MacCommandsBufferToRepeat );
                    if( MacCommandsBufferToRepeatIndex == 0 )
                    {
                        MacCommandsInNextTx = false;
                    }
                    MacCommandsBufferIndex = 0;
                }
            }

            if( ( payload != NULL ) && ( fBufferSize > 0 ) )
            {
                LoRaMacBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, fBufferSize, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, fBufferSize, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
            }
            LoRaMacBufferPktLen = pktHeaderLen + fBufferSize;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if( ( fBuffer != NULL ) && ( fBufferSize > 0 ) )
            {
                memcpy1( LoRaMacBuffer + pktHeaderLen, ( uint8_t* ) fBuffer, fBufferSize );
                LoRaMacBufferPktLen = pktHeaderLen + fBufferSize;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel )
{
    int8_t txPowerIndex = 0;
    int8_t txPower = 0;

    uint32_t sf;
    uint8_t dr_tab;
    uint32_t bw;

    txPowerIndex = LimitTxPower( ChannelsTxPower, Bands[channel.Band].TxMaxPower );
    txPower = TxPowers[txPowerIndex];

    if(ForceTxPower.Fields.Flag){
        txPower = ForceTxPower.Fields.Power;
    }

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = ChannelsDatarate;
    McpsConfirm.TxPower = txPowerIndex;

    Radio.SetChannel( channel.Frequency );

    dr_tab = LoRaMacDrTab[ChannelsDatarate];
    if(dr_tab == 0xFF){
        return LORAMAC_STATUS_DATARATE_NOT_SUPPORTED;
    }
    sf = dr_tab & 0x0F;
    bw = (dr_tab>>4) & 0x03;

    if( sf == 0 )
    { // High Speed FSK channel
        Radio.SetMaxPayloadLength( MODEM_FSK, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_FSK, txPower, 25e3, 0, 50 * 1e3, 0, 5, false, true, 0, 0, false, MAX_TX_TIME );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, bw, sf, 1, 8, false, true, 0, 0, false, MAX_TX_TIME );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    // Send now
    LORAMAC_PIN_SET();
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    LORAMAC_TIMING_TX_START();

    LoRaMacState |= MAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacInitialization( PhyType_t phytype, LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks )
{
    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;

    MulticastChannels = NULL;

    LoRaMacFlags.Value = 0;

    UpLinkCounter = 1;
    DownLinkCounter = 0;
    AdrAckCounter = 0;
    JoinRequestTrials = 0;

    NodeAckRequested = false;
    SrvAckRequested = false;

    MacCommandsInNextTx = false;
    MacCommandsBufferIndex = 0;
    MacCommandsBufferToRepeatIndex = 0;

    IsLoRaMacNetworkJoined = false;
    LoRaMacState = MAC_IDLE;

    ChannelsNbRepCounter = 0;

    MaxDCycle = 0;
    AggregatedDCycle = 1;
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    Rx1DrOffset = 0;
    RxSlot = 0;

    AckTimeoutRetries = 1;
    AckTimeoutRetriesCounter = 1;
    AckTimeoutRetry = false;

    TxTimeOnAir = 0;

    DownLinkCounterCheckingRelaxed = false;

    LoRaMacExInit(phytype);

    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );

    // Store the current initialization time
    LoRaMacInitializationTime = TimerGetCurrentTime( );

    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand1( Radio.Random( ) );

    SetPublicNetwork( PublicNetwork );
    Radio.Sleep( );

    LORAMAC_PIN_INIT();

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    int8_t datarate = ChannelsDatarate;
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;
    LoRaMacTxInfo_t txInfoTmp;

    if( txInfo == NULL )
    {
        /* skip to calculate next data rate */
        txInfo = &txInfoTmp;
    }
    else
    {
        AdrNextDr( AdrCtrlOn, false, &datarate );
    }

    txInfo->CurrentPayloadSize = MaxPayloadOfDatarate[datarate];

    txInfo->MaxPossiblePayload = 0;

    /* fOptLen never exceed minimum payload size */
    if( txInfo->CurrentPayloadSize < fOptLen )
    {
        /* it is not possible to send MAC Command anymore, clean MacComand buffer */
        LoRaMacExUpdateMacCommandMaxLen(datarate);
        fOptLen = 0;
        //return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    if( fOptLen <= LORA_MAC_COMMAND_FOPTS_MAX_LENGTH )
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - fOptLen;
        /* Port field takes one byte */
        if( txInfo->MaxPossiblePayload > 0 ){
            txInfo->MaxPossiblePayload--;
        }
    }
    else
    {
        if( size != 0 )
        {
            /* fOptLen exceeds 15 bytes, MAC command must be sent with port 0
                no more other data can be sent, return MaxPossiblePayload=0 */
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
    }

    if( ValidatePayloadLength( size, datarate, fOptLen ) == false )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    switch( mibGet->Type )
    {
#ifdef LORAMAC_FULL_MIB_PARAMETER
        case MIB_DEVICE_CLASS:
        {
            mibGet->Param.Class = LoRaMacDeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            mibGet->Param.IsNetworkJoined = IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            mibGet->Param.AdrEnable = AdrCtrlOn;
            break;
        }
        case MIB_NET_ID:
        {
            mibGet->Param.NetID = LoRaMacNetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            mibGet->Param.DevAddr = LoRaMacDevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            mibGet->Param.NwkSKey = LoRaMacNwkSKey;
            break;
        }
        case MIB_APP_SKEY:
        {
            mibGet->Param.AppSKey = LoRaMacAppSKey;
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            mibGet->Param.EnablePublicNetwork = PublicNetwork;
            break;
        }
        case MIB_CHANNELS:
        {
            mibGet->Param.ChannelList = Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = Rx2Channel;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            mibGet->Param.ChannelsMask = ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            mibGet->Param.ChannelNbRep = ChannelsNbRep;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            mibGet->Param.MaxRxWindow = MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            mibGet->Param.ReceiveDelay1 = ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            mibGet->Param.ReceiveDelay2 = ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            mibGet->Param.JoinAcceptDelay1 = JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            mibGet->Param.JoinAcceptDelay2 = JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            mibGet->Param.ChannelsDefaultDatarate = ChannelsDefaultDatarate;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            mibGet->Param.ChannelsDatarate = ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            mibGet->Param.ChannelsTxPower = ChannelsTxPower;
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            mibGet->Param.UpLinkCounter = UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            mibGet->Param.DownLinkCounter = DownLinkCounter;
            break;
        }
        case MIB_MULTICAST_CHANNEL:
        {
            mibGet->Param.MulticastList = MulticastChannels;
            break;
        }
        case MIB_DEV_EUI:
        {
            mibGet->Param.DevEui = LoRaMacDevEui;
            break;
        }
        case MIB_APP_EUI:
        {
            mibGet->Param.AppEui = LoRaMacAppEui;
            break;
        }
        case MIB_APP_KEY:
        {
            mibGet->Param.AppKey = LoRaMacAppKey;
            break;
        }
        case MIB_PHY_TYPE:
        {
            mibGet->Param.PhyType = PhyType;
            break;
        }
#else
#warning "LoRaMac Stack is optimized"
#endif
        case MIB_CHANNELS_NUM:
        {
            mibGet->Param.ChNum = CountChannelMask(ChannelsMask, 6);
            break;
        }
        case MIB_CUSTOM_RXWIN1:
        {
            uint32_t freq = 0;
            if( ChannelsMask[mibGet->Param.RxWin1.Ch/16] & (1<<(mibGet->Param.RxWin1.Ch%16)) ){
                freq = LoRaMacExGetRx1Freq(mibGet->Param.RxWin1.Ch);
            }
            mibGet->Param.RxWin1.Freq = freq;
            break;
        }
        case MIB_DR_SCHM:
        {
            uint8_t dr_val;

            dr_val = LoRaMacDrTab[mibGet->Param.DrSchm.Dr];
            mibGet->Param.DrSchm.Sf = dr_val & 0x0F;
            mibGet->Param.DrSchm.Bw = (dr_val>>4) & 0x03;
            switch(mibGet->Param.DrSchm.Bw){
            case BW125:
                mibGet->Param.DrSchm.Bw = 125;
                break;
            case BW250:
                mibGet->Param.DrSchm.Bw = 250;
                break;
            case BW500:
                mibGet->Param.DrSchm.Bw = 500;
                break;
            default:
                mibGet->Param.DrSchm.Bw = 0;
            }
            break;
        }
        case MIB_POWER_DBM:
        {
            if(ForceTxPower.Fields.Flag){
                mibGet->Param.PowerDbm = ForceTxPower.Fields.Power;
            }else{
                mibGet->Param.PowerDbm = TxPowers[ChannelsTxPower];
            }
            mibGet->Param.PowerDbm = SX1276SetRfTxPower( mibGet->Param.PowerDbm, Channels[0].Frequency, false );
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    switch( mibSet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            LoRaMacDeviceClass = mibSet->Param.Class;
            switch( LoRaMacDeviceClass )
            {
                case CLASS_A:
                {
                    // Set the radio into sleep to setup a defined state
                    Radio.Sleep( );
                    break;
                }
                case CLASS_B:
                {
                    break;
                }
                case CLASS_C:
                {
                    // Set the NodeAckRequested indicator to default
                    NodeAckRequested = false;
                    OnRxWindow2TimerEvent( );
                    break;
                }
            }
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID:
        {
            LoRaMacNetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            LoRaMacDevAddr = mibSet->Param.DevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            if( mibSet->Param.NwkSKey != NULL )
            {
                memcpy1( LoRaMacNwkSKey, mibSet->Param.NwkSKey,
                               sizeof( LoRaMacNwkSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_SKEY:
        {
            if( mibSet->Param.AppSKey != NULL )
            {
                memcpy1( LoRaMacAppSKey, mibSet->Param.AppSKey,
                               sizeof( LoRaMacAppSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            SetPublicNetwork( mibSet->Param.EnablePublicNetwork );
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            if( Radio.CheckRfFrequency(mibSet->Param.Rx2Channel.Frequency) == false)
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
            if(mibSet->Param.Rx2Channel.DrValue == DR_RFU)
            {
                if( (mibSet->Param.Rx2Channel.Datarate > LORAMAC_RX_MAX_DATARATE) || \
                    (mibSet->Param.Rx2Channel.Datarate < LORAMAC_RX_MIN_DATARATE) )
                {
                    mibSet->Param.Rx2Channel.Datarate = LORAMAC_RX_MIN_DATARATE;
                }
            }
            Rx2Channel = mibSet->Param.Rx2Channel;
            break;
        }
#if 0
        case MIB_CHANNELS_MASK:
        {
            if( mibSet->Param.ChannelsMask )
            {
                if( LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000 )
                {
                    bool chanMaskState = true;
                    if( PhyType == US915HYBRID )
                    {
                        chanMaskState = ValidateChannelMask( mibSet->Param.ChannelsMask );
                    }
                    if( chanMaskState == true )
                    {
                        if( ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) < 6 ) &&
                            ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) > 0 ) )
                        {
                            status = LORAMAC_STATUS_PARAMETER_INVALID;
                        }
                        else
                        {
                            memcpy1( ( uint8_t* ) ChannelsMask,
                                     ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( ChannelsMask ) );
                            for ( uint8_t i = 0; i < sizeof( ChannelsMask ) / 2; i++ )
                            {
                                // Disable channels which are no longer available
                                ChannelsMaskRemaining[i] &= ChannelsMask[i];
                            }
                        }
                    }
                    else
                    {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                }
                else
                {
                    memcpy1( ( uint8_t* ) ChannelsMask, ( uint8_t* ) mibSet->Param.ChannelsMask, 12 );
                }
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
#endif
        case MIB_CHANNELS_NB_REP:
        {
            if( ( mibSet->Param.ChannelNbRep >= 1 ) &&
                ( mibSet->Param.ChannelNbRep <= 15 ) )
            {
                ChannelsNbRep = mibSet->Param.ChannelNbRep;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            if( ValueInRange( mibSet->Param.ChannelsDefaultDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                ChannelsDefaultDatarate = mibSet->Param.ChannelsDefaultDatarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            if( ValueInRange( mibSet->Param.ChannelsDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                ChannelsDatarate = mibSet->Param.ChannelsDatarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsTxPower,
                              LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) )
            {
                ChannelsTxPower = mibSet->Param.ChannelsTxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            UpLinkCounter = mibSet->Param.UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            DownLinkCounter = mibSet->Param.DownLinkCounter;
            break;
        }
        case MIB_DEV_EUI:
        {
            if( mibSet->Param.DevEui != NULL )
            {
                memcpy1( LoRaMacDevEui, mibSet->Param.DevEui,
                               sizeof( LoRaMacDevEui ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_EUI:
        {
            if( mibSet->Param.AppEui != NULL )
            {
                memcpy1( LoRaMacAppEui, mibSet->Param.AppEui,
                               sizeof( LoRaMacAppEui ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_KEY:
        {
            if( mibSet->Param.AppKey != NULL )
            {
                memcpy1( LoRaMacAppKey, mibSet->Param.AppKey,
                               sizeof( LoRaMacAppKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PHY_TYPE:
        {
            PhyType = mibSet->Param.PhyType;
            break;
        }
        case MIB_CUSTOM_RXWIN1:
        {
            if( ( mibSet->Param.RxWin1.Freq == 0) ||
                (Radio.CheckRfFrequency( mibSet->Param.RxWin1.Freq ) == true) )
            {
                DlChannel[mibSet->Param.RxWin1.Ch] = mibSet->Param.RxWin1.Freq;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_POWER_DBM:
        {
            mibSet->Param.PowerDbm = SX1276SetRfTxPower( mibSet->Param.PowerDbm, Channels[0].Frequency, false );
            if(ForceTxPower.Fields.Flag){
                ForceTxPower.Fields.Power = mibSet->Param.PowerDbm;
            }else{
                int i;
                ChannelsTxPower = LORAMAC_MIN_TX_POWER;
                for( i=LORAMAC_MAX_TX_POWER; i<=LORAMAC_MIN_TX_POWER; i++ ){
                    if (mibSet->Param.PowerDbm >= TxPowers[i]) {
                        ChannelsTxPower = i;
                        break;
                    }
                }
            }
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
    bool datarateInvalid = false;
    bool frequencyInvalid = false;
    int8_t band = 0;

    // The id must not exceed LORA_MAX_NB_CHANNELS
    if( id >= LORA_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    // Validate if the MAC is in a correct state
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }
    // Validate the datarate
    if( ( params.DrRange.Fields.Min > params.DrRange.Fields.Max ) ||
        ( ValueInRange( params.DrRange.Fields.Min, LORAMAC_TX_MIN_DATARATE,
                        LORAMAC_TX_MAX_DATARATE ) == false ) ||
        ( ValueInRange( params.DrRange.Fields.Max, LORAMAC_TX_MIN_DATARATE,
                        LORAMAC_TX_MAX_DATARATE ) == false ) )
    {
        datarateInvalid = true;
    }

    // Validate the frequency
    band = LoRaMacExCheckFreq(params.Frequency);
    if( band < 0 )
    {
        frequencyInvalid = true;
    }
    else
    {
        frequencyInvalid = false;
    }

    if( ( datarateInvalid == true ) && ( frequencyInvalid == true ) )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }
    if( datarateInvalid == true )
    {
        return LORAMAC_STATUS_DATARATE_INVALID;
    }
    if( frequencyInvalid == true )
    {
        return LORAMAC_STATUS_FREQUENCY_INVALID;
    }

    // Every parameter is valid, activate the channel
    Channels[id] = params;
    Channels[id].Band = band;
    ChannelsMask[id/16] |= ( 1 << (id%16) );

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    if( id >= LORA_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    else
    {
        // Remove the channel from the list of channels
        Channels[id] = ( ChannelParams_t ){ 0, { 0 }, 0 };

        // Disable the channel as it doesn't exist anymore
        if( DisableChannelInMask( id, ChannelsMask ) == false )
        {
            return LORAMAC_STATUS_PARAMETER_INVALID;
        }
    }
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        MulticastChannels = NULL;
        return LORAMAC_STATUS_OK;
    }

    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    // Reset downlink counter
    // channelParam->DownLinkCounter = 0;

    if( MulticastChannels == NULL )
    {
        // New node is the fist element
        MulticastChannels = channelParam;
    }
    else
    {
        MulticastParams_t *cur = MulticastChannels;

        // Search the last node in the list
        while( cur->Next != NULL )
        {
            cur = cur->Next;
        }
        // This function always finds the last node
        cur->Next = channelParam;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MulticastChannels != NULL )
    {
        if( MulticastChannels == channelParam )
        {
          // First element
          MulticastChannels = channelParam->Next;
        }
        else
        {
            MulticastParams_t *cur = MulticastChannels;

            // Search the node in the list
            while( cur->Next && cur->Next != channelParam )
            {
                cur = cur->Next;
            }
            // If we found the node, remove it
            if( cur->Next )
            {
                cur->Next = channelParam->Next;
            }
        }
        channelParam->Next = NULL;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    memset1( ( uint8_t* ) &MlmeConfirm, 0, sizeof( MlmeConfirm ) );

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mlmeRequest->Type )
    {
        case MLME_JOIN:
        {
            if( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED )
            {
                return LORAMAC_STATUS_BUSY;
            }

            if( IsLoRaMacNetworkJoined == true )
            {
                return LORAMAC_STATUS_JOINED_ALREADY;
            }

            status = LoRaMacCheck();
            if(status != LORAMAC_STATUS_OK){
                return status;
            }

            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

//            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
//                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
//                ( mlmeRequest->Req.Join.AppKey == NULL ) )
//            {
//                return LORAMAC_STATUS_PARAMETER_INVALID;
//            }

            LoRaMacFlags.Bits.MlmeReq = 1;

//            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
//            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
//            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;

            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            IsLoRaMacNetworkJoined = false;

            ChannelsDatarate = AlternateDatarate( JoinRequestTrials );

            status = Send( &macHdr, 0, NULL, 0 );
            if( status == LORAMAC_STATUS_OK )
            {
                JoinRequestTrials++;
            }
            break;
        }
        case MLME_LINK_CHECK:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        default:
            break;
    }

    if( status != LORAMAC_STATUS_OK )
    {
        NodeAckRequested = false;
        LoRaMacFlags.Bits.MlmeReq = 0;
    }

    return status;
}

LoRaMacStatus_t LoRaMacCheck(void)
{
    TimerTime_t elapsedTime;
    uint8_t nbEnabledChannels;
    uint8_t delayTx;

    if( ( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING ) ||
        ( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED ) )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( ( ( IsLoRaMacNetworkJoined == false ) && ( LoRaMacJoinDutyCycle == true ) ) || ( DutyCycleOn == true ) )
    {
        LoRaMacBandTimeToValid = RegionCommonUpdateBandTimeOff();
        nbEnabledChannels = CountNbOfEnabledChannels(ChannelsMask, NULL, &delayTx);
        if( nbEnabledChannels == 0 )
        {
            if( delayTx > 0 )
            {
                return LORAMAC_STATUS_NO_BAND;
            }
            else
            {
                LoRaMacBandTimeToValid = 0;
            }
        }
        else
        {
            LoRaMacBandTimeToValid = 0;
        }
    }
    else
    {
        LoRaMacBandTimeToValid = 0;
    }

    elapsedTime = TimerGetElapsedTime( AggregatedLastTxDoneTime );
    if( AggregatedTimeOff > elapsedTime )
    {
        LoRaMacBandTimeToValid = MAX(LoRaMacBandTimeToValid, AggregatedTimeOff - elapsedTime);
        return LORAMAC_STATUS_NO_BAND;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;

    status = LoRaMacCheck();
    if(status != LORAMAC_STATUS_OK){
        return status;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mcpsRequest->Type )
    {
        case MCPS_UNCONFIRMED:
        {
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
            fPort = mcpsRequest->Req.Unconfirmed.fPort;
            fBuffer = mcpsRequest->Req.Unconfirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Unconfirmed.fBufferSize;
            //datarate = mcpsRequest->Req.Unconfirmed.Datarate;
            break;
        }
        case MCPS_CONFIRMED:
        {
            AckTimeoutRetriesCounter = 1;
            AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

            macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
            fPort = mcpsRequest->Req.Confirmed.fPort;
            fBuffer = mcpsRequest->Req.Confirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Confirmed.fBufferSize;
            //datarate = mcpsRequest->Req.Confirmed.Datarate;
            break;
        }
        case MCPS_PROPRIETARY:
        {
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            //datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    status = Send( &macHdr, fPort, fBuffer, fBufferSize );
    if( status == LORAMAC_STATUS_OK )
    {
        McpsConfirm.McpsRequest = mcpsRequest->Type;
        LoRaMacFlags.Bits.McpsReq = 1;
    }
    else
    {
        NodeAckRequested = false;
    }

    return status;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    //IsRxWindowsEnabled = enable;
}

void LoRaMacTestSetMic( uint16_t txPacketCounter )
{
    UpLinkCounter = txPacketCounter;
    //IsUpLinkCounterFixed = true;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
    if( (PhyType == EU868) || (PhyType == EU433) || (PhyType == CN779) )
    {
        DutyCycleOn = enable;
    }
}

bool LoRaMacTestGetDutyCycleOn( void )
{
    return DutyCycleOn;
}

/******************************************************************************/
// LoRaMac Extension API

/* EU868 3 */
const ChannelParams_t Eu868Channel[] = {
    { 868100000,    {((DR_5<<4)|DR_0)},     0 },
};

/* US915 */

/* CN779 3 */
const ChannelParams_t Cn780Channel[] = {
    { 779500000,    {((DR_5<<4)|DR_0)},     0 },
};

/* EU433 3 */
const ChannelParams_t Eu433Channel[] = {
    { 433175000,    {((DR_5<<4)|DR_0)},     0 },
};

/* AS923 2 */
const ChannelParams_t As923Channel[] = {
    { 923200000,    {((DR_5<<4)|DR_0)},     0 },
};

/* KR920 3 */
const ChannelParams_t Kr920Channel[] = {
    { 922100000,    {((DR_5<<4)|DR_0)},     0 },
};

/* CN470 8 */
const ChannelParams_t Cn470PrequelChannel[] = {
    { 471500000,    {((DR_5<<4)|DR_0)},     0 },
};

/* STE923  8*/
const ChannelParams_t Ste920Channel[] = {
    { 922000000,    {((DR_5<<4)|DR_0)},     0 },
};

const ChannelParams_t In865Channel[] = {
    { 865062500,    {((DR_5<<4)|DR_0)},     0 },
    { 865462500,    {((DR_5<<4)|DR_0)},     0 },
    { 865985000,    {((DR_5<<4)|DR_0)},     0 },
};

const uint16_t Eu868ChMsk[6] = { 0x0007, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
const uint16_t As923ChMsk[6] = { 0x0003, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };
const uint16_t Cn470ChMsk[6] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
const uint16_t Us915ChMsk[6] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00FF, 0x0000 };
const uint16_t Us915hybridChMsk[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000 };
const uint16_t Ste920ChMsk[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

const RegionPara_t RegionPara[] = {
    {
        //EU868,
        {869525000, DR_0, DR_RFU},
        {3, Eu868Channel, ((sizeof(Eu868Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
#ifdef LORAMAC_EU868_20DBM
        {7, 7},   // 20dBm
#warning "LoRaMac EU868 20dBm"
#else
        {5, 7},   // 16dBm
#warning "LoRaMac EU868 16dBm"
#endif
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Eu868ChMsk,
    },
    {
        //US915,
        {923300000, DR_8, DR_RFU},
        {0, NULL, 0},
        LoRaMacUs915DrTab,
        {13, 10},
        {sizeof(Us915MaxPayloadOfDatarate), Us915MaxPayloadOfDatarate},
        Us915ChMsk,
    },
    {
        //US915HYBRID,
        {923300000, DR_8, DR_RFU},
        {0, NULL, 0},
        LoRaMacUs915DrTab,
        {13, 10},
        {sizeof(Us915MaxPayloadOfDatarate), Us915MaxPayloadOfDatarate},
        Us915hybridChMsk,
    },
    {
        //CN779,
        {786000000, DR_0, DR_RFU},
        {3, Cn780Channel, ((sizeof(Cn780Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
        {2, 5},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Eu868ChMsk,
    },
    {
        //EU433,
        {434665000, DR_0, DR_RFU},
        {3, Eu433Channel, ((sizeof(Eu433Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
        {2, 5},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Eu868ChMsk,
    },
    {
        //AU915, current v1.0.2
        {923300000, DR_8, DR_RFU},
        {0, NULL, 0},
        LoRaMacAu915DrTab,
        {13, 10},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Us915ChMsk,
    },
    {
        //AU915OLD, AU915OLD
        {923300000, DR_8, DR_RFU},
        {0, NULL, 0},
        LoRaMacUs915DrTab,
        {13, 10},
        {sizeof(Us915MaxPayloadOfDatarate), Us915MaxPayloadOfDatarate},
        Us915ChMsk,
    },

    {
        //CN470,
        {505300000, DR_0, DR_RFU},
        {0, NULL, 0},
        LoRaMacCn470DrTab,
        {7, 7},
        {sizeof(Cn470MaxPayloadOfDatarate), Cn470MaxPayloadOfDatarate},
        Cn470ChMsk,
    },
    {
        //AS923,
        {923200000, DR_2, DR_RFU},
        {2, As923Channel, ((sizeof(As923Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
        {5, 7},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        As923ChMsk,
    },
    {
        //KR920,
        {921900000, DR_0, DR_RFU},
        {3, Kr920Channel, ((sizeof(Kr920Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacCn470DrTab,
        {4, 7},
        {sizeof(Kr920MaxPayloadOfDatarate), Kr920MaxPayloadOfDatarate},
        Eu868ChMsk,
    },
    {
        //IN866,
        {866550000, DR_2, DR_RFU},
        {3, In865Channel, ((sizeof(In865Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
        {13, 10},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Eu868ChMsk,
    },
    {
        //CN470PREQUEL,
        {471300000, DR_3, DR_RFU},
        {8, Cn470PrequelChannel, ((sizeof(Cn470PrequelChannel))/(sizeof(ChannelParams_t)))},
        LoRaMacCn470DrTab,
        {7, 7},
        {sizeof(Cn470MaxPayloadOfDatarate), Cn470MaxPayloadOfDatarate},
        Ste920ChMsk,
    },
    {
        //STE920,
        {923200000, DR_0, DR_RFU},
        {8, Ste920Channel, ((sizeof(Ste920Channel))/(sizeof(ChannelParams_t)))},
        LoRaMacEu868DrTab,
        {13, 10},
        {sizeof(Eu868MaxPayloadOfDatarate), Eu868MaxPayloadOfDatarate},
        Ste920ChMsk,
    },
};

void LoRaMacExEnableDefaultChannels(void)
{
    memcpy1((uint8_t *)ChannelsMask, (uint8_t *)ChannelsMaskDefault, 12);
}

void LoRaMacExSetChannel(void)
{
    int FreqStart125KHz, FreqStart500KHz;
    uint8_t dr125khz, dr500khz;
    int i;

    memset1((uint8_t *)Channels, 0, sizeof(Channels));

    if(Region->ch.buf == NULL){
        LORAMAC_START_CH_ID = 16;
        switch(PhyType){
        case CN470:
            for(i=0; i<96; i++){
                Channels[i].Frequency = 470300000 + i * 200000;
                Channels[i].DrRange.Value = ( DR_5 << 4 ) | DR_0;
                Channels[i].Band = 0;
            }
            break;
        case AU915OLD:
        case AU915:
            FreqStart125KHz = 915200000;
            FreqStart500KHz = 915900000;
            if( PhyType == AU915 ){
                dr125khz = DR_5;
                dr500khz = DR_6;
            }else{
                dr125khz = DR_3;
                dr500khz = DR_4;
            }
            goto US915_AU915_CH_INIT;
        case US915:
        case US915HYBRID:
            FreqStart125KHz = 902300000;
            FreqStart500KHz = 903000000;
            dr125khz = DR_3;
            dr500khz = DR_4;
US915_AU915_CH_INIT:
            // 125 kHz channels
            for( i = 0; i < 72 - 8; i++ ){
                Channels[i].Frequency = FreqStart125KHz + i * 200000;
                Channels[i].DrRange.Value = ( dr125khz << 4 ) | DR_0;
                Channels[i].Band = 0;
            }

            // 500 kHz channels
            for( i = 72 - 8; i < 72; i++ ){
                Channels[i].Frequency = FreqStart500KHz + ( i - ( 72 - 8 ) ) * 1600000;
                Channels[i].DrRange.Value = ( dr500khz << 4 ) | dr500khz;
                Channels[i].Band = 0;
            }
            break;
        }
    }else{
        if( Region->ch.size == 1 ){
            LORAMAC_START_CH_ID = Region->ch.num;
            for(i=0; i<Region->ch.num; i++){
                Channels[i] = Region->ch.buf[0];
                Channels[i].Frequency += 200000*i;
            }
        }else{
            LORAMAC_START_CH_ID = Region->ch.size;
            for(i=0; i<Region->ch.size; i++){
                Channels[i] = Region->ch.buf[i];
            }
        }
    }
}

uint32_t LoRaMacExGetRx1Freq(int ch)
{
    uint32_t freq;

    if( DlChannel[ch] != 0 ){
        freq = DlChannel[ch];
    }else if(LORAMAC_STEPWIDTH_RX1_CHANNEL != 0){
        freq = LORAMAC_FIRST_RX1_CHANNEL + ( ch % LORAMAC_PERODIC_RX1_CHANNEL ) * LORAMAC_STEPWIDTH_RX1_CHANNEL;
    }else{
        freq = Channels[ch].Frequency;
    }

    return freq;
}

bool LoRaMacExSetDwellTime(bool ul, bool dl)
{
    if( PhyType != AS923 ){
        return false;
    }

    memset1(MaxPayloadOfDatarate, 0, 16);
    if( ul == true ){
        LORAMAC_TX_MIN_DATARATE = DR_2;
        memcpy1(MaxPayloadOfDatarate, As923MaxPayloadOfDatarateDwell, sizeof(As923MaxPayloadOfDatarateDwell));
    }else{
        LORAMAC_TX_MIN_DATARATE = DR_0;
        memcpy1(MaxPayloadOfDatarate, Eu868MaxPayloadOfDatarate, sizeof(Eu868MaxPayloadOfDatarate));
    }

    memset1(DlMaxPayloadOfDatarate, 0, 16);
    if( dl == true ){
        LORAMAC_RX_MIN_DATARATE = DR_2;
        memcpy1(DlMaxPayloadOfDatarate, As923MaxPayloadOfDatarateDwell, sizeof(As923MaxPayloadOfDatarateDwell));
    }else{
        LORAMAC_RX_MIN_DATARATE = DR_0;
        memcpy1(DlMaxPayloadOfDatarate, Eu868MaxPayloadOfDatarate, sizeof(Eu868MaxPayloadOfDatarate));
    }
    ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;
    ChannelsDefaultDatarate = LORAMAC_DEFAULT_DATARATE;

    EirpDwellTime.Fields.UplinkDwellTime = ul;
    EirpDwellTime.Fields.DownlinkDwellTime = dl;

    LoRaMacExUpdateMacCommandMaxLen(ChannelsDatarate);

    return true;
}

int LoRaMacExSetMaxEirp(uint8_t index)
{
    int i;
    int8_t MaxEirp;

    if(index > 15){
        return -1;
    }

    EirpDwellTime.Fields.MaxEirp = index;
    MaxEirp = MaxEirpTab[index];

    for(i=0; i<=Region->power.TxPowerMax; i++){
        TxPowers[i] = MaxEirp - i*2;
    }

    for(; i<16; i++){
        TxPowers[i] = 0;
    }

    return 0;
}

void LoRaMacExInit( PhyType_t phytype )
{
    PhyType = phytype;
    Region = &RegionPara[phytype];

    LORA_MAX_NB_CHANNELS = LORA_MAX_CHS;

    LORA_MAX_NB_BANDS = 1;

    LORAMAC_TX_MIN_DATARATE = DR_0;
    LORAMAC_TX_MAX_DATARATE = DR_7;
    LORAMAC_RX_MIN_DATARATE = DR_0;
    LORAMAC_RX_MAX_DATARATE = DR_7;
    LORAMAC_DEFAULT_DATARATE = DR_0;

    LORAMAC_MIN_RX1_DR_OFFSET = 0;
    LORAMAC_MAX_RX1_DR_OFFSET = 5;

    LORAMAC_RSSI_THRESH = RSSI_FREE_TH;

    Bands[0] = (Band_t){ 100, 0, 0, 0 };

    LoRaMacDrTab = Region->drtab;

    memset1(MaxPayloadOfDatarate, 0, 16);
    memset1(DlMaxPayloadOfDatarate, 0, 16);
    memcpy1(MaxPayloadOfDatarate, Region->drpllen.buf, Region->drpllen.size);
    memcpy1(DlMaxPayloadOfDatarate, Region->drpllen.buf, Region->drpllen.size);

    /* User MaxEirp to set TxPowers table */
    LoRaMacExSetMaxEirp(Region->power.MaxEIRPIndex);
    LORAMAC_MAX_TX_POWER = 0;
    LORAMAC_MIN_TX_POWER = Region->power.TxPowerMax;
    LORAMAC_DEFAULT_TX_POWER = 0;

    LORAMAC_STEPWIDTH_RX1_CHANNEL = 0;

    ForceTxPower.Value = 0;

    LoRaMacJoinDutyCycle = true;

    switch(PhyType){
    case EU868:
        LORAMAC_DEFAULT_TX_POWER = 1;
        LORA_MAX_NB_BANDS = 5;

        Bands[0] = (Band_t){ 100 , 1, 0,  0 };
        Bands[1] = Bands[0];
        Bands[2] = Bands[0];
        Bands[3] = Bands[0];
        Bands[4] = Bands[0];
        Bands[2].DCycle = 1000;
        Bands[3].DCycle = 10;
        Bands[3].TxMaxPower = 0;
        break;
    case AU915:
        LORAMAC_DEFAULT_TX_POWER = 5;

        LORAMAC_TX_MIN_DATARATE = DR_0;
        LORAMAC_TX_MAX_DATARATE = DR_6;
		LORAMAC_RX_MIN_DATARATE = DR_8;
        LORAMAC_RX_MAX_DATARATE = DR_13;

        LORAMAC_FIRST_RX1_CHANNEL = 923300000;
        LORAMAC_LAST_RX1_CHANNEL = 927500000;
        LORAMAC_PERODIC_RX1_CHANNEL = 8;
        LORAMAC_STEPWIDTH_RX1_CHANNEL = 600000;
        break;
    case AU915OLD:
    case US915:
    case US915HYBRID:
        LORAMAC_MAX_RX1_DR_OFFSET = 3;

        LORAMAC_DEFAULT_TX_POWER = 5;

        LORAMAC_TX_MIN_DATARATE = DR_0;
        LORAMAC_TX_MAX_DATARATE = DR_4;
		LORAMAC_RX_MIN_DATARATE = DR_8;
        LORAMAC_RX_MAX_DATARATE = DR_13;

        LORAMAC_FIRST_RX1_CHANNEL = 923300000;
        LORAMAC_LAST_RX1_CHANNEL = 927500000;
        LORAMAC_PERODIC_RX1_CHANNEL = 8;
        LORAMAC_STEPWIDTH_RX1_CHANNEL = 600000;
        break;
    case CN779:
    case EU433:
        break;
    case CN470:
        LORAMAC_FIRST_RX1_CHANNEL = 500300000;
        LORAMAC_LAST_RX1_CHANNEL = 509700000;
        LORAMAC_PERODIC_RX1_CHANNEL = 48;
        LORAMAC_STEPWIDTH_RX1_CHANNEL = 200000;
    case CN470PREQUEL:
        LORAMAC_DEFAULT_TX_POWER = 0;

        LORAMAC_TX_MIN_DATARATE = DR_0;
        LORAMAC_TX_MAX_DATARATE = DR_5;
        LORAMAC_RX_MIN_DATARATE = DR_0;
        LORAMAC_RX_MAX_DATARATE = DR_5;
        break;
    case AS923:
        LORAMAC_DEFAULT_TX_POWER = 0;
        LORAMAC_DEFAULT_DATARATE = DR_2;
        LoRaMacExSetDwellTime(false, false);
        /* Cover IN865 RX1_DR_OFFSET = 7 */
    case IN865:
        LORAMAC_MAX_RX1_DR_OFFSET = 7;
        break;
    case KR920:
        LORAMAC_RSSI_THRESH = -65;
        /* Band0: 14dBm, Band1: 10dBm */
        Bands[0] = (Band_t){ 100 , 0, 0,  0 };
        Bands[1] = Bands[0];
        Bands[1].TxMaxPower = 2;
        LORAMAC_TX_MIN_DATARATE = DR_0;
        LORAMAC_TX_MAX_DATARATE = DR_5;
        LORAMAC_RX_MIN_DATARATE = DR_0;
        LORAMAC_RX_MAX_DATARATE = DR_5;
        break;
    case STE920:
        LORAMAC_DEFAULT_TX_POWER = 5;
        break;
    }

    DutyCycleOn = false;
    LoRaMacTestSetDutyCycleOn(true);

    // Reset to defaults
    LoRaMacExSetChannel();

    memcpy1((uint8_t*)ChannelsMask, (uint8_t *)Region->chmsk, 12);
    memcpy1((uint8_t*)ChannelsMaskDefault, (uint8_t *)Region->chmsk, 12);

    Rx2Channel = Region->rxwin2;
    ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;
    ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;
    ChannelsDefaultDatarate = LORAMAC_DEFAULT_DATARATE;

    MaxRxWindow = MAX_RX_WINDOW;
    ReceiveDelay1 = RECEIVE_DELAY1;
    ReceiveDelay2 = RECEIVE_DELAY2;
    JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

    ChannelsNbRep = 1;

    LoRaMacDeviceClass = CLASS_A;

    PublicNetwork = true;

    AdrCtrlOn = true;

    LoRaMacExUpdateMacCommandMaxLen(ChannelsDatarate);
}

int LoRaMacExCheckFreq(uint32_t freq)
{
    if( Radio.CheckRfFrequency( freq ) != true ){
        return BAND_UNKNOWN;
    }

    switch(PhyType){
    case EU868:
        if( ( freq >= 863000000 ) && ( freq < 865000000 ) )
        {
            return BAND_G1_2;
        }
        else if( ( freq >= 865000000 ) && ( freq <= 868000000 ) )
        {
            return BAND_G1_0;
        }
        else if( ( freq > 868000000 ) && ( freq <= 868600000 ) )
        {
            return BAND_G1_1;
        }
        else if( ( freq >= 868700000 ) && ( freq <= 869200000 ) )
        {
            return BAND_G1_2;
        }
        else if( ( freq >= 869400000 ) && ( freq <= 869650000 ) )
        {
            return BAND_G1_3;
        }
        else if( ( freq >= 869700000 ) && ( freq <= 870000000 ) )
        {
            return BAND_G1_4;
        }
        else
        {
            /* Don't check frequency band strictly, so that user could set customized channels */
            //return BAND_UNKNOWN;
        }
        break;
    case US915:
    case US915HYBRID:
    case AS923:
    case STE920:
        if( ( freq < 902000000 ) || ( freq > 928000000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    case CN779:
        if( ( freq < 779000000 ) || ( freq > 787000000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    case EU433:
        if( ( freq < 433175000 ) || ( freq > 434665000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    case AU915OLD:
    case AU915:
        if( ( freq < 915000000 ) || ( freq > 928000000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    case CN470:
    case CN470PREQUEL:
        if( ( freq < 470000000 ) || ( freq > 510000000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    case KR920:
        // Verify if the frequency is valid. The frequency must be in a specified
        // range and can be set to specific values.
        if( ( freq < 920900000 ) && ( freq > 923300000 ) ){
            return BAND_UNKNOWN;
        }
        // Range ok, check for specific value
        if( ( (freq - 920900000) % 200000 ) != 0 ){
            return BAND_UNKNOWN;
        }
        if( freq < 922000000 ){
            /* BAND_G1_0: 14dBm EIRP, BAND_G1_1: 10dBm EIRP */
            return BAND_G1_1;
        }
        break;
    case IN865:
        if( ( freq < 865000000 ) || ( freq > 867000000 ) ){
            return BAND_UNKNOWN;
        }
        break;
    }

    return BAND_G1_0;
}

void LoRaMacExUpdateMacCommandMaxLen(uint8_t datarate)
{
    uint8_t minlength;
    minlength = MaxPayloadOfDatarate[datarate];
    LORA_MAC_COMMAND_MAX_LENGTH = MIN(LORA_MAC_COMMAND_BUF_SIZE, minlength);

    /* Command is possible to be sent through port 0, save one byte for port field */
    if( LORA_MAC_COMMAND_MAX_LENGTH > LORA_MAC_COMMAND_FOPTS_MAX_LENGTH ){
        LORA_MAC_COMMAND_MAX_LENGTH--;
    }

    if( MacCommandsInNextTx && ( ( MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex) > LORA_MAC_COMMAND_MAX_LENGTH ) ){
        MacCommandsBufferIndex = 0;
        MacCommandsBufferToRepeatIndex = 0;
        MacCommandsInNextTx = false;
    }
}

////25ms RX window
//static const uint8_t LoRaSymbolTimeoutTab[]={
//    100,    //sf7/500KHz
//    50,     //sf7/250KHz
//    25,     //sf7/125KHz
//    13,     //sf8/125KHz
//    7,      //sf9/125KHz
//    5,      //sf10/125KHz
//    5,      //sf11/125KHz
//    5,      //sf12/125KHz
//};

#define USE_ERROR_10MS

#ifdef USE_ERROR_10MS
/* time error: +/-10ms  */
#define FSK_SYMBOLS                 (130)
static const int8_t LoRaSymbolTimeoutTab[]={
    84,         // sf7/500KHz
    45,         // sf7/250KHz
    25,         // sf7/125KHz
    15,         // sf8/125KHz
    10,         // sf9/125KHz
    6,          // sf10/125KHz
    6,          // sf11/125KHz
    5,          // sf12/125KHz
};
static const int8_t LoRaTimeoutOffsetTab[]={
    -10,
    -10,
    -10,
    -10,
    -10,
    0,
    0,
    0,
};
#else
/* time error: +/-5ms  */
#define FSK_SYMBOLS                 (130)
static const int8_t LoRaSymbolTimeoutTab[]={
    48,         // sf7/500KHz
    28,         // sf7/250KHz
    18,         // sf7/125KHz
    13,         // sf8/125KHz
    7,          // sf9/125KHz
    6,          // sf10/125KHz
    6,          // sf11/125KHz
    5,          // sf12/125KHz
};
static const int8_t LoRaTimeoutOffsetTab[]={
    -5,
    -5,
    -5,
    -5,
    0,
    0,
    0,
    0,
};
#endif

int8_t LoRaMacExGetRxWinTimeOffset(uint8_t dr_val)
{
    int i;
    uint8_t sf;
    uint8_t bw;

    sf = dr_val & 0x0F;
    bw = (dr_val>>4) & 0x03;

    if(sf == 0){
        i = 0;
    }else{
        i = sf-7+2-bw;
    }
    return LoRaTimeoutOffsetTab[i]+LORA_RXWIN_OFFSET;
}
uint16_t LoRaMacExGetRxWinSymbols(uint8_t sf, uint8_t bw)
{
    if(sf == 0){
        return FSK_SYMBOLS;
    }
    return LoRaSymbolTimeoutTab[sf-7+2-bw];
}

int8_t LoRaMacExSetRx1DrOft(int8_t dr)
{
    int8_t rx1dr_efct;

    switch(PhyType){
    default:
        dr = dr - Rx1DrOffset;
        if(dr<DR_0){
            dr = DR_0;
        }
        break;
    case AS923:
    case IN865:
        // TODO: it is doubt that LoRaWAN v1.0.2 has typo about MAX datarate DR_5
        if( ( Rx1DrOffset == 7 ) || ( Rx1DrOffset == 6 ) ){
            rx1dr_efct = 5 - Rx1DrOffset;
        }else{
            rx1dr_efct = Rx1DrOffset;
        }

        dr = dr - rx1dr_efct;
        if(dr<LORAMAC_RX_MIN_DATARATE){
            dr = LORAMAC_RX_MIN_DATARATE;
        }
        if(dr>DR_5){
            dr = DR_5;
        }
        break;
    case AU915:
    case AU915OLD:
    case US915:
    case US915HYBRID:
        if(dr <= LORAMAC_TX_MAX_DATARATE){
            dr += (14-LORAMAC_TX_MAX_DATARATE);
        }
        dr = dr - Rx1DrOffset;
        if(dr > DR_13){
            dr = DR_13;
        }
        if(dr<DR_8){
            dr = DR_8;
        }
        break;
    }
    return dr;
}

static int8_t AlternateDatarate( uint16_t nbTrials )
{
    int8_t datarate = LORAMAC_TX_MIN_DATARATE;

    /* US915/US915HYBRID/AU915/AU915OLD */
    if( LORAMAC_STEPWIDTH_RX1_CHANNEL == 600000 )
    {
        LoRaMacExEnableDefaultChannels();

        if( ( nbTrials & 0x01 ) == 0x01 )
        {
            datarate = LORAMAC_TX_MAX_DATARATE;
        }
        else
        {
            datarate = DR_0;
        }
    }
    else if( PhyType == AS923 )
    {
        // LoRaWAN v1.0.2
        datarate = DR_2;
    }
    else
    {
        if( ( nbTrials % 48 ) == 0 )
        {
            datarate = DR_0;
        }
        else if( ( nbTrials % 32 ) == 0 )
        {
            datarate = DR_1;
        }
        else if( ( nbTrials % 24 ) == 0 )
        {
            datarate = DR_2;
        }
        else if( ( nbTrials % 16 ) == 0 )
        {
            datarate = DR_3;
        }
        else if( ( nbTrials % 8 ) == 0 )
        {
            datarate = DR_4;
        }
        else
        {
            datarate = DR_5;
        }
    }

    return datarate;
}

bool LoRaMacExRx2FreqInRange( uint32_t freq )
{
    if( LoRaMacExCheckFreq( freq ) < 0 ){
        return false;
    }

    if( LORAMAC_STEPWIDTH_RX1_CHANNEL != 0 ){
        if(( freq >= LORAMAC_FIRST_RX1_CHANNEL ) &&
           ( freq <= LORAMAC_LAST_RX1_CHANNEL ) &&
           ( ( ( freq - LORAMAC_FIRST_RX1_CHANNEL ) % LORAMAC_STEPWIDTH_RX1_CHANNEL ) == 0 ) )
        {
            return true;
        }else{
            return false;
        }
    }

    return true;
}

int LoRaMacExPack(uint8_t *buf, uint8_t *pl, int len, LoRaMacExPackPara_t *para)
{
    int i, j;
    uint8_t *key;

    i=0;
    buf[i++] = para->mhdr.Value;

    buf[i++] = (uint8_t)(para->devaddr>>0);
    buf[i++] = (uint8_t)(para->devaddr>>8);
    buf[i++] = (uint8_t)(para->devaddr>>16);
    buf[i++] = (uint8_t)(para->devaddr>>24);

    buf[i++] = (uint8_t)(para->fctrl.Value);

    buf[i++] = (uint8_t)(para->fcnt>>0);
    buf[i++] = (uint8_t)(para->fcnt>>8);

    for(j=0; j < para->fctrl.Bits.FOptsLen; j++){
        buf[i++] = para->fopts[j];
    }

    if( ( i + len ) > LORAMAC_PHY_MAXPAYLOAD ){
        return -1;
    }

    if( len != 0 ){
        buf[i++] = para->fport;
        if( para->fport == 0 ){
            key = LoRaMacNwkSKey;
        }
        else{
            key = LoRaMacAppSKey;
        }
        LoRaMacPayloadEncrypt( pl, len, key, para->devaddr, DOWN_LINK, para->fcnt, buf+i );
        i+=len;
    }

    uint32_t mic;
    LoRaMacComputeMic( buf, i, LoRaMacNwkSKey, para->devaddr, DOWN_LINK, para->fcnt, &mic );

    if( ( i + 4 ) > LORAMAC_PHY_MAXPAYLOAD ){
        return -1;
    }

    buf[i++] = mic & 0xFF;
    buf[i++] = ( mic >> 8 ) & 0xFF;
    buf[i++] = ( mic >> 16 ) & 0xFF;
    buf[i++] = ( mic >> 24 ) & 0xFF;

    return i;
}

typedef struct{
    uint8_t cmd;
    uint8_t len;
}LoRaMacExMacCommandTab_t;

const LoRaMacExMacCommandTab_t LoRaMacExMoteMacCommandTab[]={
    { MOTE_MAC_LINK_CHECK_REQ,          MOTE_MAC_LEN_LINK_CHECK_REQ         },
    { MOTE_MAC_LINK_ADR_ANS,            MOTE_MAC_LEN_LINK_ADR_ANS           },
    { MOTE_MAC_DUTY_CYCLE_ANS,          MOTE_MAC_LEN_DUTY_CYCLE_ANS         },
    { MOTE_MAC_RX_PARAM_SETUP_ANS,      MOTE_MAC_LEN_RX_PARAM_SETUP_ANS     },
    { MOTE_MAC_DEV_STATUS_ANS,          MOTE_MAC_LEN_DEV_STATUS_ANS         },
    { MOTE_MAC_NEW_CHANNEL_ANS,         MOTE_MAC_LEN_NEW_CHANNEL_ANS        },
    { MOTE_MAC_RX_TIMING_SETUP_ANS,     MOTE_MAC_LEN_RX_TIMING_SETUP_ANS    },
    { MOTE_MAC_TX_PARAM_SETUP_ANS,      MOTE_MAC_LEN_TX_PARAM_SETUP_ANS     },
    { MOTE_MAC_DL_CHANNEL_ANS,          MOTE_MAC_LEN_DL_CHANNEL_ANS         },
};

int8_t LoRaMacExGetMoteMacCommandLen(uint8_t cmd)
{
    int j;
    for(j=0; j<(sizeof(LoRaMacExMoteMacCommandTab)/sizeof(LoRaMacExMacCommandTab_t)); j++){
        if( LoRaMacExMoteMacCommandTab[j].cmd == cmd ){
            return LoRaMacExMoteMacCommandTab[j].len;
        }
    }
    return -1;
}

bool LoRaMacExCheckMoteMacCommand(uint8_t *cmd, uint8_t len)
{
    uint8_t i;
    int8_t cmdlen;

    i=0;
    while(i<len){
        cmdlen = LoRaMacExGetMoteMacCommandLen(cmd[i]);
        if(cmdlen<0){
            return false;
        }else if( (i+cmdlen) == len ){
            return true;
        }
        i += cmdlen;
    }
    return true;
}

bool LoRaMacExCheckSrvMacCommand(uint8_t *cmd, uint8_t len)
{
    uint8_t i;

    i=0;
    while(i<len){
#ifdef USE_DEBUGGER
        //printf("CMD: %02x\n", cmd[i]);
#endif
        switch( cmd[i] ){
            case SRV_MAC_DEV_STATUS_REQ:
                // 0 byte payload
                i += 1;
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
            case SRV_MAC_RX_TIMING_SETUP_REQ:
            case SRV_MAC_TX_PARAM_SETUP_REQ:
                // 1 byte payload
                i += 2;
                break;
            case SRV_MAC_LINK_CHECK_ANS:
                // 2 bytes payload
                i += 3;
                break;
            case SRV_MAC_LINK_ADR_REQ:
            case SRV_MAC_RX_PARAM_SETUP_REQ:
            case SRV_MAC_DL_CHANNEL_REQ:
                // 4 byte payload
                i += 5;
                break;
            case SRV_MAC_NEW_CHANNEL_REQ:
                // 5 byte payload
                i += 6;
                break;
            default:
                return false;
        }
        if( i == len ){
            return true;
        }
    }

    return false;
}




