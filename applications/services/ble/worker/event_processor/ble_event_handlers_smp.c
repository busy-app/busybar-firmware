
#include "../ble_worker_i.h"

#define TAG "BleSMPEvent"

bool ble_event_handler_smp_response(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);

    BleWorker* instance = context;
    if(ble_device_is_paired(instance->device)) {
        BLE_LOG_W("Device is paired, remove previous pairing!");
        ble_device_disconnect(instance->device);
    } else {
        ble_device_response_pairing_capabilities(instance->device);
    }

    return true;
}

bool ble_event_handler_smp_encrypt_started(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("%s", __func__);

    BleWorker* instance = context;
    rsi_bt_event_encryption_enabled_t* enc_enabled = data;
    ble_device_handle_encryption_start(instance->device, enc_enabled);

    ble_device_connection_update(
        instance->device,
        instance->event_loop,
        (BleConnectionUpdateParametersDoneCallback)ble_worker_invoke_connect_callback,
        instance);
    return true;
}

bool ble_event_handler_smp_ltk_request(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);
    BleWorker* instance = context;

    do {
        if(!ble_device_send_encryption_response(instance->device)) break;

        if(ble_device_is_paired(instance->device)) break;

        ble_device_request_pairing(instance->device);
    } while(false);

    return true;
}

bool ble_event_handler_smp_security_keys(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("%s", __func__);
    BleWorker* instance = context;
    BleSecurityData* security_data = ble_device_get_security_data(instance->device);
    rsi_bt_event_le_security_keys_t* rsi_ble_event_le_security_keys = data;
    ble_security_set_rpa_data(security_data, rsi_ble_event_le_security_keys);

    do {
        if(!ble_security_rpa_enable(security_data)) break;

        if(!ble_security_save_data(security_data)) {
            BLE_LOG_W("Failed to save Security");
            break;
        }

        BLE_LOG_I("RPA keys saved");
    } while(false);

    return true;
}

bool ble_event_handler_smp_pairing_failed(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);
    BleWorker* instance = context;

    return ble_device_disconnect(instance->device);
}
