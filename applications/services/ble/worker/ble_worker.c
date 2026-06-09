#include "ble_worker_i.h"
#include <furi.h>

#include "ble_worker_util.h"

#define TAG "BleWorker"

///TODO: Remove after all connection issues will be resolved
// Uncomment macro below in order to force ble advertising with public address only
// #define BLE_DEBUG_ADVERTISE_FORCE_PUBLIC

#define BLE_DEFAULT_LOCAL_NAME "BUSY Bar"

//===========================================================================================
///TODO:Remove this in future
static BleWorker* ble_worker_instance = NULL;
//===========================================================================================

static void retry_phy_timer_callback(void* ctx) {
    BleWorker* instance = ctx;
    ble_incoming_nwp_event_processor_spawn_event(
        instance->event_proc, BleIncomingNwpEventTypeDataLengthChange, 0, NULL);
}
//===========================================================================================

static int32_t ble_worker_thread_callback(void* context) {
    BleWorker* instance = context;
    BLE_LOG_I("Worker Thread Start");
    ble_device_start(instance->device);

    instance->event_loop = furi_event_loop_alloc();

    ble_transmitter_subscribe(instance->transport, instance->event_loop, context);
    ble_incoming_nwp_event_processor_subscribe(instance->event_proc, instance->event_loop);

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

    instance->on_connection_changed_cb = connect_callback;
    instance->on_connection_changed_ctx = ctx;
    //----------------------------------------------------------------------------------------------------------------

    instance->event_proc = ble_incoming_nwp_event_processor_alloc(instance);
    instance->transport = ble_transmitter_alloc();
    ble_nwp_core_config_callbacks(instance->event_proc, instance->transport);
    //----------------------------------------------------------------------------------------------------------------

    instance->device = ble_device_alloc(instance->transport);
    ble_device_set_name(instance->device, BLE_DEFAULT_LOCAL_NAME);
    ///TODO: this is to keep old code working
    instance->security_data = ble_device_get_security_data(instance->device);
    instance->pairing_info_available = ble_device_is_paired(instance->device);

    instance->retry_phy_timer =
        furi_timer_alloc(retry_phy_timer_callback, FuriTimerTypeOnce, instance);

    //Appearance adjustment
    uuid_t uuid = {0};
    uuid.size = 2;
    uuid.val.val16 = 0x2A01;
    uint16_t value_handle = 0;
    if(ble_find_characteristic_value_handle_by_uuid(&uuid, 0x001E, &value_handle)) {
        uint16_t data = 0x00C0;
        BLE_LOG_D("Handle found: %04X", value_handle);
        sl_status_t status = rsi_ble_set_local_att_value(value_handle, 2, (uint8_t*)&data);
        UNUSED(status);
        BLE_LOG_D("Status: %lX", status);
    }
    //---------------------------------------
    ///TODO:Remove this in future
    ble_worker_instance = instance;
    //---------------------------------------
    return instance;
}

bool ble_worker_register_service(BleServiceObject* service) {
    return ble_device_register_service(ble_worker_instance->device, service);
}

void ble_worker_start() {
    do {
        ///TODO: Maybe some checks of thread state
        furi_thread_start(ble_worker_instance->thread);
    } while(false);
}

void ble_worker_stop() {
    if(ble_worker_instance) {
        FuriThreadState state = furi_thread_get_state(ble_worker_instance->thread);
        if(state == FuriThreadStateRunning) {
            ble_incoming_nwp_event_processor_spawn_event(
                ble_worker_instance->event_proc, BleIncomingNwpEventTypeExit, 0, NULL);

            furi_thread_join(ble_worker_instance->thread);
            BLE_LOG_I("BLE Stopped");
        }
    }
}

void ble_worker_send(uint16_t handle, uint16_t data_size, const uint8_t* data, uint16_t cccd_value) {
    ble_device_send_data(ble_worker_instance->device, handle, data_size, data, cccd_value);
}

void ble_worker_receive_confirm(uint16_t handle, uint8_t cccd_value) {
    ble_device_receive_confirm(ble_worker_instance->device, handle, cccd_value);
}

bool ble_worker_forget_pairing() {
    return ble_device_forget_paired(ble_worker_instance->device);
}

bool ble_worker_pairing_exists() {
    return ble_device_is_paired(ble_worker_instance->device);
}

void ble_worker_set_name(const char* new_name) {
    furi_assert(new_name);

    ble_device_set_name(ble_worker_instance->device, new_name);
}
