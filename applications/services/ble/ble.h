#pragma once

#include "ble_callback_types.h"
#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

typedef enum {
    BleServiceStateReset, /*Service was just created. Will move to BleServiceStateInitialization when it will create all inner objects*/
    BleServiceStateInitialization, /* Service performs initialization sequence for all inner ble services.
    U5 also sends init data to 917 to help him create its services */
    BleServiceStateReady, /*All init sequences are done. All inner services configured, and both u5 and 917 ready to work. But ble still disabled*/
    BleServiceStateAdvertising, /*User enabled ble, device start advertising.*/
    BleServiceStateConnected, /*Remote device connected to bsb over ble*/
    BleServiceStateError, /*Error occured.*/

    BleServiceStateCount, /*Total amount of states. Used in some cyclic operations*/
} BleServiceState;

typedef enum {
    BlePairingStateUnkown,
    BlePairingStateNotPaired,
    BlePairingStatePaired,

    BlePairingStateCount
} BlePairingState;

typedef enum {
    BleUartChannelNordic,
    BleUartChannelHM10,

    BleUartChannelCount
} BleUartChannel;

typedef struct Ble Ble;

BleServiceState ble_get_state(Ble* ble);

bool ble_start(Ble* ble);

bool ble_stop(Ble* ble);

BlePairingState ble_pairing_get_state(Ble* ble);

bool ble_forget(Ble* ble);

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
