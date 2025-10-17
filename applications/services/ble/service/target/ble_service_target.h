#pragma once

#include "../ble_service_i.h"

bool ble_service_target_execute(
    BleServiceObject* instance,
    BleIntercomFrameType frame_type,
    BleCommand command,
    size_t data_size,
    void* data);
