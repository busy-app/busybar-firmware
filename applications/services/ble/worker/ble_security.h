#pragma once

#include "rsi_ble.h"
#include "rsi_ble_apis.h"
#include "rsi_ble_common_config.h"
#include "rsi_bt_common_apis.h"
#include <furi.h>

typedef struct BleSecurityData BleSecurityData;

const rsi_bt_event_encryption_enabled_t* ble_security_get_pairing_data(BleSecurityData* security);
void ble_security_set_pairing_data(
    BleSecurityData* security,
    const rsi_bt_event_encryption_enabled_t* encryption);

const rsi_bt_event_le_security_keys_t* ble_security_get_rpa_data(BleSecurityData* security);
void ble_security_set_rpa_data(
    BleSecurityData* security,
    const rsi_bt_event_le_security_keys_t* rpa_keys);

bool ble_security_save_data(const BleSecurityData* const security);
bool ble_security_delete_data(BleSecurityData* security);

bool ble_security_rpa_enable(BleSecurityData* security);
bool ble_security_rpa_disable();

bool ble_security_init(BleSecurityData** instance);

void ble_security_respond_with_keys(BleSecurityData* instance);
