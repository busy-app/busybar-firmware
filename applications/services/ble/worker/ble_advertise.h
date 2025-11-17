#pragma once

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
 * @param new_name which will be used for advertise. Max_length = Advertise_pack_max_length - Used_space.
 * If name exceeds max_length it will be trimmed like "Name..." 
 * Now Max_length = 31 - 17 = 14
 */
void ble_advertise_set_name(BleAdvertiseContext* instance, const char* new_name);

/**
 * @brief Refresh advertise config on 2nd core
 * @param instance of advertise context
 */
void ble_advertise_refresh_data(const BleAdvertiseContext* instance);

/**
 * @brief Print current config into logs
 * @param instance of advertise context
 */
void ble_advertise_print_data(const BleAdvertiseContext* instance);
