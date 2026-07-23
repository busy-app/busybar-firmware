/**
 * @file ble.h
 * @brief Public ble api
 */
#pragma once

#include "ble_callback_types.h"
#include <furi.h>

/**
 * @brief BLE FURI record identifier.
 */
#define RECORD_BLE "ble"

/**
 * @brief Defines max data size for characteristics. Used by @ref "ble_http_repeater.h" and @ref "ble_streaming.h"
 */
#define MAX_TX_CHUNK_SIZE (237)

/**
 * @brief Ble device address string size
 */
#define BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE (18)

/** Enumeration of ble service statuses */
typedef enum {
    BleServiceStatusReset, /**< Service was just created. Will move to BleServiceStatusInitialization when it will create all inner objects*/
    BleServiceStatusInitialization, /**< Service performs initialization sequence for all inner ble services.
    U5 also sends init data to 917 to help him create its services */
    BleServiceStatusReady, /**< All init sequences are done. All inner services configured, and both u5 and 917 ready to work. But ble is still disabled*/
    BleServiceStatusAdvertising, /**< Ble enabled, device not paired and is visible to all.*/
    BleServiceStatusConnectable, /**< Ble enabled, device is paired and waits for remote device to connect.*/
    BleServiceStatusConnected, /**< Remote device connected to bsb over ble*/
    BleServiceStatusError, /**< Error occurred.*/

    BleServiceStatusCount, /**< Total amount of states. Used in some cyclic operations*/
} BleServiceStatus;

/** Enumeration of ble uart channels which public ble uart api sets in order to read/write data */
typedef enum {
    BleUartChannelNordic, /**< Points to Nordic Uart ble service, which is used by @ref "ble_http_repeater.h" */
    BleUartChannelHM10, /**< Points to HM10 Uart ble service, which is used by @ref "ble_streaming.h" */

    BleUartChannelCount /**< Total amount of channels */
} BleUartChannel;

/** Depicts current ble state */
typedef struct {
    BleServiceStatus status; /**< Status shows service state */
    uint8_t remote_device_address
        [BLE_REMOTE_DEVICE_ADDRESS_STRING_SIZE]; /**< Shows connected device address when connected, otherwise there will be zeros*/
} BleState;

/**
 * @brief Opaque Ble type declaration.
 */
typedef struct Ble Ble;

/**
 * @brief Get ble state. This is a blocking API.
 *
 * @param[in] ble pointer to the ble instance
 * @param[in,out] output state struct filled with current ble state.
 * @returns true on success, otherwise false
 */
bool ble_get_state(Ble* ble, BleState* const output);

/**
 * @brief Enable ble by sending BleCommandEnable. This is a blocking API.
 *
 * After this command device starts advertising with its name and public 
 * or random address depending on pairing state.
 *
 * @param[in] ble pointer to the ble instance
 * @returns true on success, otherwise false
 */
bool ble_start(Ble* ble);

/**
 * @brief Disable ble by sending BleCommandDisable. This is a blocking API.
 *
 * @param[in] ble pointer to the ble instance
 * @returns true on success, otherwise false
 */
bool ble_stop(Ble* ble);

/**
 * @brief Forget current pairing with remote device. This is a blocking API.
 *
 * @param[in] ble pointer to the ble instance
 * @returns true on success, otherwise false
 */
bool ble_forget(Ble* ble);

/**
 * @brief Get pubsub in order to subscribe on ble events.
 *
 * @param[in] ble pointer to the ble instance
 * @returns pubsub instance for further use
 */
FuriPubSub* ble_get_pubsub(Ble* ble);

/**
 * @brief Register callback function for ble uart rx characteristic.
 *
 * Callback and context can be set only once, after that any attempts to set new 
 * callback will be ignored until NULL value will be provided for callback and context,
 * this will reset callback and new callback can be set after
 * 
 * @param[in] ble pointer to the ble instance
 * @param[in] channel channel to which callback will be set
 * @param[in] rx_cb pointer to callback function
 * @param[in] ctx pointer to callback function context
 */
void ble_uart_set_rx_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataUpdatedCallback rx_cb,
    void* ctx);

/**
 * @brief Register callback function for ble uart tx characteristic.
 *
 * @param[in] ble pointer to the ble instance
 * @param[in] channel channel to which callback will be set
 * @param[in] tx_done_cb pointer to callback function
 * @param[in] ctx pointer to callback function context
 */
void ble_uart_set_tx_done_callback(
    Ble* ble,
    BleUartChannel channel,
    BleDataTransmitDoneCallback tx_done_cb,
    void* ctx);

/**
 * @brief Send data over ble uart.
 *
 * Copies payload to tx characteristic of choosen uart channel and triggers 
 * ble service to do the rest. This is non-blocking API, synchronisation 
 * primitives must be used externally in order to wait for
 * transmission complete. Before that a proper callback must be registered using 
 * @ref ble_uart_set_tx_done_callback. 
 *
 * @param[in] ble pointer to the ble instance
 * @param[in] channel channel to which callback will be set
 * @param[in] data payload to be send
 * @param[in] data_size payload size
 */
void ble_uart_tx_data(Ble* ble, BleUartChannel channel, const void* data, const size_t data_size);

/**
 * @brief Register session callback function for Nordic Uart service
 *
 * This api is used for Nordic Uart channel only
 *
 * @param[in] ble pointer to the ble instance
 * @param[in] session_update_cb pointer to callback function from @ref "ble_callback_types.h"
 * @param[in] ctx pointer to callback function context
 */
void ble_uart_set_session_callback(Ble* ble, BleDataUpdatedCallback session_update_cb, void* ctx);

/**
 * @brief Set session value for Nordic Uart Session characteristic
 *
 * This api is used for Nordic Uart channel. This is non-blocking API, 
 * and it works in same way as @ref ble_uart_tx_data
 *
 * @param[in] ble pointer to the ble instance
 * @param[in] session new session value
 */
void ble_uart_session_set_value(Ble* ble, const uint32_t session);
