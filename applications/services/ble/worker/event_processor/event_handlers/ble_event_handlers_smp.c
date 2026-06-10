
#include "ble_event_handlers_smp.h"

#include "../../ble_worker_i.h"

#define TAG "BleSMPEvent"

bool ble_event_handler_smp_response(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);

    BleWorker* instance = context;
    ble_device_response_pairing_capabilities(instance->device);

    return true;
}

bool ble_event_handler_smp_encrypt_started(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("ble_event_handler_smp_encrypt_started");
    BleWorker* instance = context;
    rsi_bt_event_encryption_enabled_t* enc_enabled = data;
    ble_security_set_pairing_data(instance->security_data, enc_enabled);

    if(instance->pairing_info_available == 0) {
        instance->pairing_info_available = 1;

        if(ble_security_save_data(instance->security_data))
            BLE_LOG_I("Security data saved");
        else
            BLE_LOG_W("Failed to save Security");
    }
    return true;
}

bool ble_event_handler_smp_ltk_request(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("ble_event_handler_smp_ltk_request");
    BleWorker* instance = context;
    // rsi_bt_event_smp_resp_t* resp = data;
    // memcpy(&instance->rsi_bt_event_smp_resp, resp, data_size);

    ///TODO: Move this logic to ble_security module
    sl_status_t status;

    if(instance->pairing_info_available) {
        const rsi_bt_event_encryption_enabled_t* encrypt_keys =
            ble_security_get_pairing_data(instance->security_data);

        status = rsi_ble_ltk_req_reply(
            instance->remote_dev_address,
            (1 | encrypt_keys->enabled | (encrypt_keys->sc_enable << 7)),
            encrypt_keys->localltk);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("ltk req reply cmd failed with reason = %lx", status);
        }
        BLE_LOG_I("Paired device");
        bool connected = ble_device_is_connected(instance->device);
        instance->on_connection_changed_cb(
            instance->on_connection_changed_ctx, connected, instance->str_remote_address);

        // ble_worker_spawn_event(
        //     instance->event_queue, BleWorkerEventTypeAdjustConnectionRequest, 0, NULL);
    } else {
        BLE_LOG_I("Not paired device");
        status = rsi_ble_ltk_req_reply(instance->remote_dev_address, 0, NULL);
        if(status != RSI_SUCCESS) {
            BLE_LOG_W("ltk negative req reply cmd failed with reason = %lx \n", status);
        }
    }

    return true;
}

bool ble_event_handler_smp_security_keys(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    BLE_LOG_I("ble_event_handler_smp_security_keys");
    BleWorker* instance = context;
    rsi_bt_event_le_security_keys_t* rsi_ble_event_le_security_keys = data;
    ble_security_set_rpa_data(instance->security_data, rsi_ble_event_le_security_keys);

    do {
        if(!ble_security_rpa_enable(instance->security_data)) break;

        if(!ble_security_save_data(instance->security_data)) {
            BLE_LOG_W("Failed to save Security");
            break;
        }

        bool connected = ble_device_is_connected(instance->device);
        instance->on_connection_changed_cb(
            instance->on_connection_changed_ctx, connected, instance->str_remote_address);
        BLE_LOG_I("Security keys saved");
    } while(false);

    return true;
}

bool ble_event_handler_smp_pairing_failed(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("ble_event_handler_smp_pairing_failed");
    BleWorker* instance = context;

    sl_status_t status = rsi_ble_disconnect((int8_t*)instance->remote_dev_address);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("failed disconnect status = %lx \n", status);
    }

    return true;
}
