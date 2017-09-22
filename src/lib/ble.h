/*
    _/_/_/    _/_/_/    _/_/_/  _/_/_/  _/      _/    _/_/_/  _/    _/  _/_/_/_/
   _/    _/    _/    _/          _/    _/_/    _/  _/        _/    _/  _/
  _/_/_/      _/      _/_/      _/    _/  _/  _/  _/  _/_/  _/_/_/_/  _/_/_/
 _/    _/    _/          _/    _/    _/    _/_/  _/    _/  _/    _/  _/
_/    _/  _/_/_/  _/_/_/    _/_/_/  _/      _/    _/_/_/  _/    _/  _/
    (C)2016 RisingHF, all rights reserved.
*/
#ifndef __BLE_H
#define __BLE_H

typedef struct{
    int8_t rssi;
    uint8_t uuid[6];
}ble_pkt_t;

void ble_init(void);
int ble_sta_handle(void);
int ble_busy(void);
int8_t ble_new(void);
void ble_get_uuid(uint8_t *buf, int len);
int8_t ble_get_rssi(void);
int ble_readable(void);
int ble_read(uint8_t *uuid, int8_t *rssi);

#endif
