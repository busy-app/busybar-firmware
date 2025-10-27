#pragma once

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include <furi.h>

typedef struct {
    rsi_bt_event_le_security_keys_t irk;
    rsi_bt_event_encryption_enabled_t ltk;
} BleSecurityData;

// bool ble_security_load_data(BleSecurityData* security);
bool ble_security_save_data(const BleSecurityData* const security);
bool ble_security_delete_data();

bool ble_security_rpa_enable(rsi_bt_event_le_security_keys_t* rpa_keys);
bool ble_security_rpa_disable();

bool ble_security_init(BleSecurityData* instance);
