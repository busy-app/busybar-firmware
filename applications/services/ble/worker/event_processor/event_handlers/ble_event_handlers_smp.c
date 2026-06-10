
#include "ble_event_handlers_smp.h"

#include "../../ble_worker_i.h"

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
    return true;
}

bool ble_event_handler_smp_ltk_request(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    BLE_LOG_I("%s", __func__);
    BleWorker* instance = context;

    do {
        if(!ble_device_send_encryption_response(instance->device)) break;

        if(ble_device_is_paired(instance->device)) {
            bool connected = ble_device_is_connected(instance->device);
            BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
            FuriString* addr = furi_string_alloc();
            BleDeviceBase* peer = ble_connection_get_peer(conn);
            ble_device_base_format_address(peer, BleDeviceAddressTypeOrigin, addr);
            BLE_LOG_I("Paired device: %s", furi_string_get_cstr(addr));

            instance->on_connection_changed_cb(
                instance->on_connection_changed_ctx,
                connected,
                (const uint8_t*)furi_string_get_cstr(addr));
            furi_string_free(addr);
        }
    } while(false);
    // ble_worker_spawn_event(
    //     instance->event_queue, BleWorkerEventTypeAdjustConnectionRequest, 0, NULL);

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

        bool connected = ble_device_is_connected(instance->device);

        BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
        FuriString* addr = furi_string_alloc();
        BleDeviceBase* peer = ble_connection_get_peer(conn);
        ble_device_base_format_address(peer, BleDeviceAddressTypeOrigin, addr);

        instance->on_connection_changed_cb(
            instance->on_connection_changed_ctx,
            connected,
            (const uint8_t*)furi_string_get_cstr(addr));
        furi_string_free(addr);

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
