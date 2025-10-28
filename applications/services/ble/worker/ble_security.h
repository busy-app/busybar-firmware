#pragma once

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include <furi.h>

/**
 * @brief Security bundle
 */
typedef struct BleSecurityData BleSecurityData;

/**
 * @brief Get inner pairing struct.
 *
 * @param security security instance
 * 
 * @return Pairing data
 */
const rsi_bt_event_encryption_enabled_t* ble_security_get_pairing_data(BleSecurityData* security);

/**
 * @brief Set new pairing data to struct.
 *
 * @param security security instance
 * @param encryption new pairing data received from ble stack during pairing process
 */
void ble_security_set_pairing_data(
    BleSecurityData* security,
    const rsi_bt_event_encryption_enabled_t* encryption);

/**
 * @brief Get inner rpa identity struct.
 *
 * @param security security instance
 * 
 * @return identity data
 */
const rsi_bt_event_le_security_keys_t* ble_security_get_rpa_data(BleSecurityData* security);

/**
 * @brief Set new rpa identity data to struct.
 *
 * @param security security instance
 * @param rpa_keys new rpa data received from ble stack during pairing process
 */
void ble_security_set_rpa_data(
    BleSecurityData* security,
    const rsi_bt_event_le_security_keys_t* rpa_keys);

/**
 * @brief Allocate security bundle struct at startup
 */
BleSecurityData* ble_security_alloc();

/**
 * @brief Free bundle
 */
void ble_security_free(BleSecurityData* instance);

/**
 * @brief Loads data from nvm and initializes RPA security logic during Ble startup init
 */
bool ble_security_init(BleSecurityData* instance);

/**
 * @brief Save current security bundle to nvm
 *
 * @param security security instance
 * 
 * @return True if success, otherwise false
 */
bool ble_security_save_data(const BleSecurityData* const security);

/**
 * @brief Remove all ble data from nvm and reset security bundle
 *
 * @param security security instance
 * 
 * @return True if success, otherwise false
 */
bool ble_security_delete_data(BleSecurityData* security);

/**
 * @brief Enables RPA logic when pairing is finished and when startup. 
 *
 * @param security security instance
 * 
 * @return True if success, otherwise false
 */
bool ble_security_rpa_enable(BleSecurityData* security);

/**
 * @brief Disables RPA logic, before forgeting previous pairing
 * 
 * @return True if success, otherwise false
 */
bool ble_security_rpa_disable();
