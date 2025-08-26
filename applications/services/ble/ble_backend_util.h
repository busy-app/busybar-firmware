#pragma once
#include <furi.h>

void ble_print_service_hierarchy(uint16_t last_handle);
bool ble_find_characteristic_value_handle_by_uiid(
    const uuid_t* uuid,
    uint16_t last_handle,
    uint16_t* output_handle);
