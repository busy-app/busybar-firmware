#pragma once

#include "../ble.h"

/**
 * @brief Opaque BleHttpRepeater type declaration.
 */
typedef struct BleHttpRepeater BleHttpRepeater;

/**
 * @brief Allocates http repeater instance for future use.
 *
 * @param[in] ble Pointer to Ble service.
 * @param[out] BleHttpRepeater* Pointer to http instance.
 */
BleHttpRepeater* ble_http_repeater_alloc(Ble* ble);

/**
 * @brief Deletes http instance and free all internals.
 *
 * @param[in] instance Pointer to http instance.
 */
void ble_http_repeater_free(BleHttpRepeater* instance);

/**
 * @brief Update http instance state according to current Ble status.
 * Http will start if status equals @ref BleServiceStatusConnected, in other cases it will be stopped.
 *
 * @param[in] instance Pointer to http instance.
 * @param[in] status Current status of Ble service
 */
void ble_http_repeater_update(BleHttpRepeater* instance, const BleServiceStatus status);
