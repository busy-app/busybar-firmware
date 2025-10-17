#pragma once

#include "ble_state.h"
#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

#define BLE_AUTO_INIT

typedef enum {
    BleUartChannelNordic,
    BleUartChannelHM10,

    BleUartChannelCount
} BleUartChannel;

typedef struct Ble Ble;

bool ble_init(Ble* ble);

BleServiceState ble_get_state(Ble* ble);

bool ble_start(Ble* ble);

bool ble_stop(Ble* ble);

void ble_uart_set_rx_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataUpdatedCallback rx_cb,
    void* ctx);

void ble_uart_set_tx_done_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataTransmitDoneCallback tx_done_cb,
    void* ctx);

void ble_uart_tx_data(Ble* ble, BleUartChannel channel, const void* data, const size_t data_size);
