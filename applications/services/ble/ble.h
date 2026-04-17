#pragma once

#include "ble_callback_types.h"
#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

#define MAX_TX_CHUNK_SIZE (237)

#define BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE (18)

typedef enum {
    BleServiceStatusReset, /*Service was just created. Will move to BleServiceStatusInitialization when it will create all inner objects*/
    BleServiceStatusInitialization, /* Service performs initialization sequence for all inner ble services.
    U5 also sends init data to 917 to help him create its services */
    BleServiceStatusReady, /*All init sequences are done. All inner services configured, and both u5 and 917 ready to work. But ble still disabled*/
    BleServiceStatusAdvertising, /*Ble enabled, device not paired and is visible to all.*/
    BleServiceStatusConnectable, /*Ble enabled, device is paired and waits for remote device to connect.*/
    BleServiceStatusConnected, /*Remote device connected to bsb over ble*/
    BleServiceStatusError, /*Error occured.*/

    BleServiceStatusCount, /*Total amount of states. Used in some cyclic operations*/
} BleServiceStatus;

typedef enum {
    BleUartChannelNordic,
    BleUartChannelHM10,

    BleUartChannelCount
} BleUartChannel;

typedef struct {
    BleServiceStatus status;
    uint8_t remote_device_address[BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE];
} BleState;

typedef struct Ble Ble;

bool ble_get_state(Ble* ble, BleState* const output);

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

void ble_uart_set_session_callback(Ble* ble, BleDataUpdatedCallback session_update_cb, void* ctx);

void ble_uart_session_set_value(Ble* ble, const uint32_t session);
