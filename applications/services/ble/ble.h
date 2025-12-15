#pragma once

#include "ble_callback_types.h"
#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

#define BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE (18)

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

typedef struct {
    BleServiceState state;
    BlePairingState pairing;
    uint8_t remote_device_address[BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE];
} BleStatus;

typedef struct Ble Ble;

bool ble_get_status(Ble* ble, BleStatus* const output);

bool ble_start(Ble* ble);

bool ble_stop(Ble* ble);

bool ble_forget(Ble* ble);

FuriPubSub* ble_get_pubsub(Ble* ble);

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
