#pragma once

#include "ble_device_base.h"

typedef void (*BleConnectionUpdateParametersDoneCallback)(void* ctx);

typedef struct {
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
} BleConnectionTimings;

typedef struct {
    /**Maximum TX Octets to be transmitted*/
    uint16_t MaxTxOctets;
    /**Maximum TX time to transmit the MaxTxOctets*/
    uint16_t MaxTxTime;
    /**Maximum Rx Octets to be received*/
    uint16_t MaxRxOctets;
    /**Maximum Rx time to receive the MaxRxOctets*/
    uint16_t MaxRxTime;
} BleConnectionDataLength;

typedef union {
    struct {
        bool phy_le_1m    : 1;
        bool phy_le_2m    : 1;
        bool phy_le_coded : 1;
        uint8_t reserved  : 5;
    } flags;
    uint8_t value;
} BlePhy;

typedef struct BleConnectionContext BleConnectionContext;

BleConnectionContext*
    ble_connection_alloc(BleDeviceAddressType type, const uint8_t* const peer_address);
void ble_connection_free(BleConnectionContext* instance);

BleDeviceBase* ble_connection_get_peer(BleConnectionContext* instance);

bool ble_connection_update_phy_and_data_length_by_timer(BleConnectionContext* instance);

const BleConnectionTimings* ble_connection_get_timings(BleConnectionContext* instance);
void ble_connection_set_timings(
    BleConnectionContext* instance,
    const BleConnectionTimings* const timings);

const BleConnectionDataLength* ble_connection_get_data_length(BleConnectionContext* instance);
void ble_connection_set_data_length(
    BleConnectionContext* instance,
    const BleConnectionDataLength* const data_length);

const BlePhy* ble_connection_get_tx_phy(BleConnectionContext* instance);

const BlePhy* ble_connection_get_rx_phy(BleConnectionContext* instance);
void ble_connection_set_phy(
    BleConnectionContext* instance,
    const uint8_t tx_phy,
    const uint8_t rx_phy);

void ble_connection_start_update_parameters(
    BleConnectionContext* instance,
    FuriEventLoop* event_loop,
    BleConnectionUpdateParametersDoneCallback done_cb,
    void* ctx);
