/**
 * @file ble_streaming.h
 * @brief API for streaming device state and screen over ble
 *
 * Streams data provided by state publisher to HM10 Uart characteristics
 */
#pragma once

#include "../ble.h"

/**
 * @brief Opaque BleStreaming type declaration.
 */
typedef struct BleStreaming BleStreaming;

/**
 * @brief Allocates streaming instance for future use.
 *
 * @param[in] ble Pointer to Ble service.
 * @return pointer to streaming instance.
 */
BleStreaming* ble_streaming_alloc(Ble* ble);

/**
 * @brief Deletes streaming instance and free all internals.
 *
 * @param[in] instance Pointer to streaming instance.
 */
void ble_streaming_free(BleStreaming* instance);

/**
 * @brief Update streaming instance state according to current Ble status.
 * Streaming will start if status equals @ref BleServiceStatusConnected, in other cases it will be stopped.
 *
 * @param[in] instance Pointer to streaming instance.
 * @param[in] status Current status of Ble service
 */
void ble_streaming_update(BleStreaming* instance, const BleServiceStatus status);
