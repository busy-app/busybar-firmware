#pragma once

#include "../ble_service_i.h"

bool ble_service_target_init(BleServiceObject* instance);

bool ble_service_target_write(BleServiceObject* instance);

bool ble_service_target_read(BleServiceObject* instance);

void ble_service_target_notify(
    BleServiceObject* instance,
    uint8_t ch_index,
    void* data,
    size_t data_size);

bool ble_service_target_process_request(BleServiceObject* instance);

bool ble_service_target_process_response(BleServiceObject* instance);

// typedef bool (*BleServiceProcessRequestFrameByStateCallback)();

// static bool dummy() {
// }

// static const BleServiceProcessRequestFrameByStateCallback request_handlers[] = {
//     [BleServiceStateReset] = dummy,
//     [BleServiceStateInitialization] = ble_service_target_init,
//     [BleServiceStateReady] = dummy,
//     [BleServiceStateAdvertising] = dummy,
//     [BleServiceStateConnected] = dummy,
//     [BleServiceStateError] = dummy,
// };
