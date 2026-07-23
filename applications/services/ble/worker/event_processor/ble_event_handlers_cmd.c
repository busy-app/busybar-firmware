#include "../ble_worker_i.h"

#define TAG "BleEventCmd"

bool ble_event_handler_cmd_exit(size_t data_size, void* data, void* context) {
    BLE_LOG_D("ble_event_handler_cmd_exit");
    UNUSED(data_size);
    UNUSED(data);
    BleWorker* instance = context;

    if(ble_device_stop(instance->device)) {
        furi_event_loop_stop(instance->event_loop);
    }
    return true;
}
