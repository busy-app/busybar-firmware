/**
 * @file ble_receiver.h
 * @brief Responsible for receiving data from remote
 */
#pragma once

#include <furi.h>
#include "../../service/target/ble_service_target.h"

/**
 * @brief Opaque BleReceiverContext type declaration.
 */
typedef struct BleReceiverContext BleReceiverContext;

/**
 * @brief Allocate receiver instance
 * @param[in] peer_addr address of the connected remote device
 * @return pointer to created instance
 */
BleReceiverContext* ble_receiver_alloc(const uint8_t* peer_addr);

/**
 * @brief Free receiver instance
 * @param[in] instance receiver instance to be destroyed
 */
void ble_receiver_free(BleReceiverContext* instance);

/**
 * @brief Enables receiver
 * @param[in] instance receiver instance
 */
void ble_receiver_enable(BleReceiverContext* instance);

/**
 * @brief Processes write request from nwp
 * @param[in] instance receiver instance
 * @param[in] service service which characteristic is requested
 * @param[in] char_index index of characteristic in service
 * @param[in] handle handle of characteristic
 * @param[in] data_size size of data nwp wants to write into characteristic
 * @param[in] data data nwp wants to write into characteristic
 * @return true if data were written, otherwise false
 */
bool ble_receiver_process_write_request(
    BleReceiverContext* instance,
    BleServiceObject* service,
    const uint8_t char_index,
    const uint16_t handle,
    const size_t data_size,
    const void* data);

/**
 * @brief Send confirmation that write request is done
 *
 * @param[in] instance receiver instance
 * @param[in] handle handle of characteristic
 * @param[in] cccd_value property flags for characteristic
 */
void ble_receiver_transfer_confirm(
    BleReceiverContext* instance,
    uint16_t handle,
    uint8_t cccd_value);
