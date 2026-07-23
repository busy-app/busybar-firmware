/**
 * @file ble_callback_types.h
 * @brief Type definitions for callbacks used by services and public api @ref "ble.h" to indicate update and transmit operations
 */
#pragma once

#include <furi.h>

/**
 * @brief Type of callback to be triggered when new data received by ble characteristic.
 *
 * @param[in] data_size size of received payload
 * @param[in] data pointer to array with received data
 * @param[in] context to be used for this callback
 */
typedef void (*BleDataUpdatedCallback)(size_t data_size, void* data, void* context);

/**
 * @brief Type of callback to be triggered when characteristic finished data transmission.
 *
 * @param[in] context to be used for this callback
 */
typedef void (*BleDataTransmitDoneCallback)(void* context);
