/**
 * @file ble_worker.h
 * @brief Responsible for all operations with ble and exists only on 917
 */
#pragma once
#include "../service/ble_service.h"

#define BLE_REMOTE_ADDRESS_STRING_SIZE (18)

/**
 * @brief Opaque BleWorker type declaration.
 */
typedef struct BleWorker BleWorker;

/**
 * @brief Type definition for connection state change. 
 *
 * Worker will trigger this callback if connection state was changed
 *
 * @param[in] context used for this call
 * @param[in] connected actual connection state
 * @param[in] remote_dev_address address of connected remote device
 */
typedef void (
    *BleConnectionStateChanged)(void* context, bool connected, const uint8_t* remote_dev_address);

/**
 * @brief Initializes worker when BleCommandInit is process by 917 at startup.
 *
 * @param[in] connect_callback callback which will be used by worker for connection events
 * @param[in] ctx context for connect_callback
 * @returns pointer to the instance of BleWorker
 */
BleWorker* ble_worker_init(BleConnectionStateChanged connect_callback, void* ctx);

/**
 * @brief Performs registration of each inner ble service during service initialization
 *
 * During this process each service and characteristics inside are registered in nwp 
 * and get their own handles which are then used as indexes during requesting those fields
 * 
 * @param[in] service instance to service object
 * @returns true if registration success
 */
bool ble_worker_register_service(BleServiceObject* service);

/**
 * @brief Send payload to characteristic set by handle
 * 
 * @param[in] handle unique pointer to particular ble characteristic. BleServiceObject and all inner 
 * BleCharacteristicObject get such handle during ble_worker_register_service call
 * @param[in] data_size size of payload to be send
 * @param[in] data payload
 * @param[in] cccd_value special field with properties which each characteristic in ble has
 */
void ble_worker_send(uint16_t handle, uint16_t data_size, const uint8_t* data, uint16_t cccd_value);

/**
 * @brief Used by services in order to confirm data receiving
 *
 * This function is called only by U5 response that it received data
 * @param[in] handle unique pointer to particular ble characteristic
 * @param[in] cccd_value special field with properties which each characteristic in ble has
 */
void ble_worker_receive_confirm(uint16_t handle, uint8_t cccd_value);

/**
 * @brief Starts ble on @ref BleCommandEnable from U5
 */
void ble_worker_start();

/**
 * @brief Stops ble on @ref BleCommandDisable from U5
 */
void ble_worker_stop();

/**
 * @brief Forgets pairing with remote device on @ref BleCommandForgetPairing
 * @returns true when pairing removed
 */
bool ble_worker_forget_pairing();

/**
 * @brief Checks whether pairing with remote device exists.
 *
 * This function returns value nevertheless of actual connection state.
 * @returns true when paired, otherwise false
 */
bool ble_worker_pairing_exists();

/**
 * @brief Sets new device name used for advertising
 * @param[in] new_name name got from U5 side
 */
void ble_worker_set_name(const char* new_name);
