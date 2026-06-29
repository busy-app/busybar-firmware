#pragma once

#include "../_nwp_callbacks/ble_nwp_headers.h"

#include <furi.h>

/**
 * @brief Advertise context instance
 */
typedef struct BleAdvertiseContext BleAdvertiseContext;

/**
 * @brief Allocate advertise context
 */
BleAdvertiseContext* ble_advertise_alloc();

/**
 * @brief Free advertise context
 * @param instance of advertise context
 */
void ble_advertise_free(BleAdvertiseContext* instance);

/**
 * @brief Set new name, which will be used for advertise
 *
 * @param instance of advertise context
 * @param new_name which will be used for advertise.
 */
void ble_advertise_set_name(BleAdvertiseContext* instance, const char* new_name);

/**
 * @brief Start advertise
 * @param instance of advertise context
 * @param advertise_to_paired_only if device is paired it will result in anonymous advertise config
 * @param key RPA key structure used to configure accept list when pairing is present
 */
bool ble_advertise_start(
    BleAdvertiseContext* instance,
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key);

/**
 * @brief Stop advertise
 * @param instance of advertise context
 */
bool ble_advertise_stop(BleAdvertiseContext* instance);

/**
 * @brief Print current config into logs
 * @param instance of advertise context
 */
void ble_advertise_print_data(const BleAdvertiseContext* instance);
