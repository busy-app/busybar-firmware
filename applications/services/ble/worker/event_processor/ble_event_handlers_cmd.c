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

bool ble_event_handler_cmd_forget_paired(size_t data_size, void* data, void* context) {
    BLE_LOG_D("ble_event_handler_cmd_forget_paired");
    UNUSED(data_size);
    UNUSED(data);
    BleWorker* instance = context;

    bool result = ble_device_forget_paired(instance->device);
    BleDeviceState state = ble_device_get_state(instance->device);

    if(state != BleDeviceStateForgetting) {
        instance->pending_command->result = result;
        api_lock_unlock(instance->pending_command->api_lock);
        instance->pending_command = NULL;

        ble_worker_invoke_disconnect_callback(instance);
    }

    return true;
}
