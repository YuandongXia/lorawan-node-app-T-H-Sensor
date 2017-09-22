# Changelog

# v2.1.15 vs v2.1.14
+ Fix AT+LW=SCR
+ Fix AT+TEST=LWDL

# v2.1.14 vs v2.1.13
+ Fix join duty error

# v2.1.13 vs v2.1.12
+ Fix join duty cycle error if more channels are enabled

# v2.1.12 vs v2.1.11
+ Fix MULTICAST support
+ Fix PROPRIETARY frame downlink
+ Fix class c mode join
+ Fix RX Window timing

# V2.1.11 vs v2.1.10
+ Fix LoRaWAN FSK receive
+ Fix AS923 RXWIN2 receive
+ LoRaMac fix unexpected DR error
+ LoRaMac fix FSK receive error
+ Fix AT+TEST=LWDL,CMSG/MSG inverted issue

# v2.1.10 vs v2.1.9
+ Fix AT+DELAY
+ Fix US915/US915HYBRID RXWINDOW

# v2.1.9 vs v2.1.8
## LoRaWAN
+ Add MAC command validator
+ Fix CN470PREQUEL RXWIN1 channel
+ Fix ADR command for all band
+ Fix RXWIN2 command
+ Fix payload length error issue
+ Fix KR920 output power control
+ Fix DisableChannelInMask
+ SRV_MAC_TX_PARAM_SETUP_REQ only available at AS923

## CMD
+ Add +MSG: Length error %d, %d means maximum payload length allowed.
+ Fix AT+TEST=TXCW / AT+TEST=TXCLORA

# v2.1.8 vs v2.1.7
+ Update AT+CH command. Fix channel mask issue.
+ Add AT+LW=LEN command to get maximum available payload length
+ Add AT+LW=JDC command to control Join dutycycle, set "AT+LW=JDC,OFF" to disable JoinDutyCycle.
	AT+LW=DC has higher priority than AT+LW=JDC.
+ Fix LoRaWAN RX1DROffset range issue
+ Fix LoRaWAN payload length issue
+ AT+MSG/CMSG/MSGHEX/CMSGHEX no longer echo “+xxx: TX "xxxxx "
+ AT+MSG/CMSG/MSGHEX/CMSGHEX returns compact format hex string
+ AT+KEY=type,key returns compact key string
+ AT+TEST=RFCFG returns NET instead of PNET

# v2.1.7 vs v2.1.5
+ Fix AT+TEST=TXCW and AT+TEST=TXCLORA issue
+ Add US915OLD/IN865
+ Update TX power table
+ Fix CN779/EU433
+ EU868/CN779/EU433 band Duty Cycle is enabled by default
+ Optimize AT+RXWIN2 command

# v2.1.5 vs v2.1.3
+ Rename serveral band plan, EU434 -> EU433, CN780 -> CN779, AU920 -> AU915
+ Rename AT+CH=MASK to AT+CH=NUM
+ Fix AT+TEST=ID,appeui,deveui, foce overwrite current DEVEUI and APPEUI
+ Add "AT+UART=BR,baudrate" (baudrate: 9600 14400 19200 38400 57600 76800 115200 230400)
+ Update AT+POWER
	+ AT+POWER check channel 0 frequency to decide MAX power (CH 0 should never be off)
	+ Rename AT+POWER=SCHEME to AT+POWER=TABLE
	+ Add "AT+POWER=pow,FORCE" use to force LoRaWAN use a fixed output power
+ Fix AT+DR crash the program issue after ADR is triggered
+ Fix SNR print issue

# v2.1.3 vs v2.1.2
+ Add AT+LW=SCR command, this command is used to by pass MAX_FCNT_GAP cheking, if it is on MAX_FCNT_GAP will be bypass, has security risk.
+ Fix AT+CH=ch,freq,drmin,drmax
+ Fix AT+LW=MC
+ Fix AT+DELAY
+ Fix AT+UART=TIMEOUT

# V2.1.2 VS v2.0.10

## Customer

### New
+ AT+DR=band will reset all lorawan parameters (channels, datarate, rxwin2, rxwin1)
+ AT+WDT=ON/OFF to enanble or disable watchdog, need reset to make it valid, default ON. Sleep current on WDT is 2uA
+ Support Fpending event
+ Update AT+FDEFAULT, AT+FDEFAULT and AT+FDEFAULT=RISINGHF both valid, even AT+FDEFAULT=xxxxx is also valid
+ AT+UART=TIMEOUT returns "+UART: TIMEOUT, 0" if timeout is disabled
+ Supports band plan EU868, EU434, US915, US915HYBRID, CN780, CN470, AS923, KR920, CN470PREQUEL
+ Supports TxParamSetupReq/TxParamSetupAns and DlChannelReq/DlChannelAns (All LoRaWAN 1.0.2 Class A/C Mac commands are supported)
+ Support maximum 96 channels
+ Add AT+CH=MASK command
+ Add AT+POWER=SCHEME
+ Add AT+LW=TPS

### Fix
+ Fix AT+RXWIN2=freq,sf,bw command doesn't work issue

### Not compitable
+ Remove AT+RXWIN1  ON/OFF control, AT+RXWIN1=ch,freq to overwrite default rxwin1 frequency
+ Remove AT+DR=CUSTOM and AT+DR=CUSTOM, DRx, ..., CUSTOM band plan is no longer supported
+ Remove AT+REG command
+ Remove AT+TEST=HELP
+ Change AT+TEST=RSSI behiviour
+ Remove AT+HELP
+ Update AT+LW=CDR, as CUSTOM band is no longer supported, set CDR is disabled, check CDR is still valid
+ AT+TEST=LWDL returns "+TEST: LWDL DONE" instead of "+TEST: LORAWAN DOWNLINK TX DONE"
+ Add AT+TEST=RFCFG options, CRC/IQ/PNET is configurable
	+ Returns `+TEST: RFCFG F:434000000, SF7, BW125K, TXPR:8, RXPR:8, POW:20dBm, CRC:OFF, IQ:ON, PNET:ON`
+ AT+DFU=ON no longer returns "Enter bootloader after reboot in 5s..."
+ No longer prints MACCMD
+ Class C downlink force return MSG type
+ Class C downlink force return "+MSG:Done", to make it easier for user to parse

## Internal
+ Add "AT+TEST=RXCW, AT+TEST=RXCW,freq,cnt" to measure RSSI and FEI
+ Add "AT+TEST=GPIO, AT+TEST=GPIO,pinmap,pinval" command to test gpio
+ Add `AT+TEST=ID`, `AT+TEST=ID,appeui,deveui`
	+ `AT+TEST=ID` returns STM32 MCU UUID 96bits and default APPEUI and DEVEUI
	+ `AT+TEST=ID,appeui,deveui` could be used to burn IEEE APPEUI and DEVEUI
