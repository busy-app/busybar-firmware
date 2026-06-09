#pragma once

#include "../ble_service_i.h"

bool ble_service_target_execute(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleServiceCommandEnum command,
    size_t data_size,
    const void* data);

bool ble_service_write_char_data_or_cccd_by_handle(
    BleServiceObject* instance,
    uint8_t index,
    const uint16_t handle,
    const void* data,
    const size_t data_size);
