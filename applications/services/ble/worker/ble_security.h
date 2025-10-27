#pragma once

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include <furi.h>

// #ifdef RSI_BLE_RESOLVING_LIST_SIZE
// #undef RSI_BLE_RESOLVING_LIST_SIZE
// #define RSI_BLE_RESOLVING_LIST_SIZE 1
// #endif

typedef struct {
    rsi_bt_event_le_security_keys_t irk;
    rsi_bt_event_encryption_enabled_t ltk;
} BleSecurityData;

void ble_sercurity_format_rpa_data(
    FuriString* output,
    const rsi_bt_event_le_security_keys_t* security);

void ble_security_print_encryption_data(
    FuriString* output,
    const rsi_bt_event_encryption_enabled_t* encryption);

bool ble_security_load_data(BleSecurityData* security);
bool ble_security_save_data(const BleSecurityData* const security);
bool ble_security_delete_data();

bool ble_security_rpa_enable(rsi_bt_event_le_security_keys_t* rpa_keys);
bool ble_security_rpa_disable();
