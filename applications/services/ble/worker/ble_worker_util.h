#pragma once
#include <furi.h>
#include "rsi_ble_apis.h"

void ble_print_service_hierarchy(void);
bool ble_find_characteristic_value_handle_by_uuid(
    const uuid_t* uuid,
    uint16_t last_handle,
    uint16_t* output_handle);

void ble_worker_util_log_payload(
    const uint16_t handle,
    const uint8_t chunk_num,
    const uint8_t* data,
    const size_t data_size);
