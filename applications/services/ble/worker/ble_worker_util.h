#pragma once
#include <furi.h>
#include "rsi_ble_apis.h"

void ble_print_service_hierarchy(void);
bool ble_find_characteristic_value_handle_by_uiid(
    const uuid_t* uuid,
    uint16_t last_handle,
    uint16_t* output_handle);
