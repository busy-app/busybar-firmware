#include "ble_worker_i.h"

#include "_nwp_callbacks/ble_nwp_core_callbacks.h"

#include <furi.h>

#include "ble_worker_util.h"

#define TAG "BleWorker"

#define BLE_DEFAULT_LOCAL_NAME "BUSY Bar"

static int32_t ble_worker_thread_callback(void* context) {
    BleWorker* instance = context;
    BLE_LOG_I("Worker Thread Start");
    ble_device_start(instance->device);

    instance->event_loop = furi_event_loop_alloc();

    ble_transmitter_subscribe(instance->transport, instance->event_loop, context);
    ble_incoming_nwp_event_processor_subscribe(instance->event_proc, instance->event_loop);

    api_lock_unlock(instance->api_lock);
    furi_event_loop_run(instance->event_loop);

    ble_incoming_nwp_event_processor_unsubscribe(instance->event_proc, instance->event_loop);
    ble_transmitter_unsubscribe(instance->transport, instance->event_loop);

    furi_event_loop_free(instance->event_loop);

    return 0;
}

BleWorker* ble_worker_init(BleConnectionStateChanged connect_callback, void* ctx) {
    furi_assert(connect_callback);
    furi_assert(ctx);

    BleWorker* instance = malloc(sizeof(BleWorker));
    instance->thread =
        furi_thread_alloc_ex("BleWorker", 3072U, ble_worker_thread_callback, instance);
    instance->api_lock = api_lock_alloc_locked();

    instance->on_connection_changed_cb = connect_callback;
    instance->on_connection_changed_ctx = ctx;
    //----------------------------------------------------------------------------------------------------------------

    instance->event_proc = ble_incoming_nwp_event_processor_alloc(instance);
    instance->transport = ble_transmitter_alloc();

    instance->device = ble_device_alloc(instance->transport);
    ble_device_set_name(instance->device, BLE_DEFAULT_LOCAL_NAME);

    ble_nwp_core_config_callbacks(instance->event_proc, instance->transport);
    //----------------------------------------------------------------------------------------------------------------

    return instance;
}

void ble_worker_invoke_connect_callback(BleWorker* instance) {
    FuriString* addr = furi_string_alloc();

    BleConnectionContext* conn = ble_device_get_connection_context(instance->device);
    BleDeviceBase* peer = ble_connection_get_peer(conn);
    ble_device_base_format_address(peer, BleDeviceAddressTypeOrigin, addr);

    instance->on_connection_changed_cb(
        instance->on_connection_changed_ctx, true, (const uint8_t*)furi_string_get_cstr(addr));
    furi_string_free(addr);
}

void ble_worker_invoke_disconnect_callback(BleWorker* instance) {
    uint8_t dummy[BLE_REMOTE_ADDRESS_STRING_SIZE] = {0};
    instance->on_connection_changed_cb(instance->on_connection_changed_ctx, false, dummy);
}

bool ble_worker_register_service(BleWorker* instance, BleServiceObject* service) {
    furi_assert(instance);
    return ble_device_register_service(instance->device, service);
}

void ble_worker_start(BleWorker* instance) {
    furi_assert(instance);
    FuriThreadState state = furi_thread_get_state(instance->thread);
    if(state == FuriThreadStateRunning) return;

    api_lock_relock(instance->api_lock);
    furi_thread_start(instance->thread);
    api_lock_wait_unlock(instance->api_lock);
}

void ble_worker_stop(BleWorker* instance) {
    furi_assert(instance);

    FuriThreadState state = furi_thread_get_state(instance->thread);
    if(state == FuriThreadStateRunning) {
        ble_incoming_nwp_event_processor_spawn_event(
            instance->event_proc, BleIncomingNwpEventTypeExit, 0, NULL);

        furi_thread_join(instance->thread);
        BLE_LOG_I("BLE Stopped");
    }
}

void ble_worker_send(
    BleWorker* instance,
    uint16_t handle,
    uint16_t data_size,
    const uint8_t* data,
    uint16_t cccd_value) {
    furi_assert(instance);
    ble_device_send_data(instance->device, handle, data_size, data, cccd_value);
}

void ble_worker_receive_confirm(BleWorker* instance, uint16_t handle, uint8_t cccd_value) {
    furi_assert(instance);
    ble_device_receive_confirm(instance->device, handle, cccd_value);
}

bool ble_worker_forget_pairing(BleWorker* instance) {
    furi_assert(instance);
    if(ble_device_is_connected(instance->device)) {
        ble_device_disconnect(instance->device);
    }
    return ble_device_forget_paired(instance->device);
}

bool ble_worker_pairing_exists(BleWorker* instance) {
    furi_assert(instance);
    return ble_device_is_paired(instance->device);
}

void ble_worker_set_name(BleWorker* instance, const char* new_name) {
    furi_assert(instance);
    furi_assert(new_name);

    ble_device_set_name(instance->device, new_name);
}
