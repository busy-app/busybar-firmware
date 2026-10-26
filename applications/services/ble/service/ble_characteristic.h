/**
 * @file ble_characteristic.h
 * @brief Characteristic logic
 */
#pragma once

#include "ble_service.h"
#include "ble_service_config_types.h"

/**
 * @brief Opaque BleCharacteristicObject type declaration.
 */
typedef struct BleCharacteristicObject BleCharacteristicObject;

/**
 * @brief Creates characteristic from predefined config.
 * This method is called during service allocation
 * @param[in] config Characteristic config
 * @param[in] parent_service Service which stores this characteristic
 * @return pointer to characteristic instance
 */
BleCharacteristicObject* ble_characteristic_alloc(
    const BleCharacteristicConfig* config,
    BleServiceObject* parent_service);

/**
 * @brief Free characteristic.
 * @param[in] instance pointer to characteristic instance
 */
void ble_characteristic_free(BleCharacteristicObject* instance);

/**
 * @brief Reset characteristic internals.
 * @param[in] instance pointer to characteristic instance
 */
void ble_characteristic_reset(BleCharacteristicObject* instance);

/**
 * @brief Get pointer to internal data buffer.
 * @param[in] instance pointer to characteristic instance
 * @return pointer to internal data buffer
 */
const void* ble_characteristic_get_data(BleCharacteristicObject* instance);

/**
 * @brief Get actual data size.
 * Size can be less than maximum buffer size
 * @param[in] instance pointer to characteristic instance
 * @return Actual size of data in buffer
 */
size_t ble_characteristic_get_data_size(BleCharacteristicObject* instance);

/**
 * @brief Set data to characteristic.
 * This marks the characteristic as locally modified. The caller must enqueue
 * parent service processing, or use ble_service_write_data(), to transmit the data.
 * @param[in] instance pointer to characteristic instance
 * @param[in] data pointer to buffer with new data
 * @param[in] data_size size of new data
 */
void ble_characteristic_set_data(
    BleCharacteristicObject* instance,
    const void* data,
    const size_t data_size);

/**
 * @brief Get characteristic modification status.
 * Characteristic can be modified from both sides
 * Locally - when this side stores data to characteristic for further transmission
 * Remote - when new data were received from other side via intercom
 * @param[in] instance pointer to characteristic instance
 * @return true if modified, otherwise false
 */
bool ble_characteristic_is_modified(BleCharacteristicObject* instance);

/**
 * @brief Get pointer to characteristic config struct.
 * @param[in] instance pointer to characteristic instance
 * @return pointer to characteristic config struct
 */
const BleCharacteristicConfig* ble_characteristic_get_config(BleCharacteristicObject* instance);

/**
 * @brief Store characteristic handle retrieved during registration process.
 * @param[in] instance pointer to characteristic instance
 * @param[in] handle handle value returned by NWP during registration
 */
void ble_characteristic_set_handle(BleCharacteristicObject* instance, uint16_t handle);

/**
 * @brief Get characteristic handle.
 * @param[in] instance pointer to characteristic instance
 * @return handle value for this characteristic
 */
uint16_t ble_characteristic_get_handle(BleCharacteristicObject* instance);

/**
 * @brief Store CCCD handle retrieved during registration process.
 * @param[in] instance pointer to characteristic instance
 * @param[in] cccd_handle handle value for CCCD field returned by NWP during registration
 */
void ble_characteristic_set_cccd_handle(BleCharacteristicObject* instance, uint16_t cccd_handle);

/**
 * @brief Check if provided handle is CCCD or not.
 * @param[in] instance pointer to characteristic instance
 * @param[in] possible_cccd handle which need to be tested if it represents CCCD field or not
 * @return true if handle is CCCD otherwise false
 */
bool ble_characteristic_is_cccd_handle(BleCharacteristicObject* instance, uint16_t possible_cccd);

/**
 * @brief Set new CCCD value for characteristic.
 * @param[in] instance pointer to characteristic instance
 * @param[in] value new CCCD value
 */
void ble_characteristic_set_cccd_value(BleCharacteristicObject* instance, uint8_t value);

/**
 * @brief Get current CCCD value from characteristic.
 * @param[in] instance pointer to characteristic instance
 * @return CCCD value
 */
uint8_t ble_characteristic_get_cccd_value(BleCharacteristicObject* instance);

/**
 * @brief Pack characteristic data into intercom frame before sending to other side.
 * @param[in] instance pointer to characteristic instance
 * @param[in] output pointer to intercom frame with data packed from characteristic
 * @return intercom data size 
 */
size_t ble_characteristic_encode(BleCharacteristicObject* instance, BleCharacteristicData* output);

/**
 * @brief Decode characteristic data from intercom frame received from other side.
 * @param[in] instance pointer to characteristic instance
 * @param[in] input pointer to intercom frame with data for characteristic
 * @return true if packet was decoded successfully
 */
bool ble_characteristic_decode(
    BleCharacteristicObject* instance,
    const BleCharacteristicData* input);

/**
 * @brief Register update callback.
 * Callback will be triggered when new data were received 
 * @param[in] instance pointer to characteristic instance
 * @param[in] callback callback function
 * @param[in] ctx callback context
 */
void ble_characteristic_register_update_callback(
    BleCharacteristicObject* instance,
    BleDataUpdatedCallback callback,
    void* ctx);

/**
 * @brief Register transmission done callback.
 * Callback will be triggered when data stored in characteristic
 * were successfully received and processed by other side
 * @param[in] instance pointer to characteristic instance
 * @param[in] callback callback function
 * @param[in] ctx callback context
 */
void ble_characteristic_register_tx_done_callback(
    BleCharacteristicObject* instance,
    BleDataTransmitDoneCallback callback,
    void* ctx);
