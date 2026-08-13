#include "ble_incoming_nwp_event_processor.h"
#include "ble_incoming_nwp_event.h"

#include "ble_event_handlers.h"

#include "../../ble_log.h"

#define TAG "BleEvent"

struct BleIncomingNwpEventProcessor {
    bool run;
    FuriMutex* lock;
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    void* context;
};

typedef bool (*BleWorkerEventHandler)(size_t data_size, void* data, void* context);

bool ble_event_handler_dummy(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    UNUSED(context);
    furi_crash("Unknown event!");
}

static const BleWorkerEventHandler event_handlers[BleIncomingNwpEventTypeCount] = {
    [BleIncomingNwpEventTypeUnknown] = ble_event_handler_dummy,
    [BleIncomingNwpEventTypeExit] = ble_event_handler_cmd_exit,
    [BleIncomingNwpEventTypeForgetPaired] = ble_event_handler_cmd_forget_paired,
    [BleIncomingNwpEventTypeConnected] = ble_event_handler_gap_connected,
    [BleIncomingNwpEventTypeDisconnected] = ble_event_handler_gap_disconnected,
    [BleIncomingNwpEventTypePhyUpdateComplete] = ble_event_handler_gap_phy_update_complete,
    [BleIncomingNwpEventTypeConnUpdate] = ble_event_handler_gap_connection_update,
    [BleIncomingNwpEventTypeDataLengthChange] = ble_event_handler_gap_length_change,

    [BleIncomingNwpEventTypeReceiveRemoteFeatures] = ble_event_handler_gap_receive_remote_features,
    [BleIncomingNwpEventTypeReadRequest] = ble_event_handler_gatt_read_request_event,
    [BleIncomingNwpEventTypeWrite] = ble_event_handler_gatt_write_event,
    [BleIncomingNwpEventTypeDataTransmit] = ble_event_handler_dummy,
    [BleIncomingNwpEventTypeMtu] = ble_event_handler_gatt_mtu,

    [BleIncomingNwpEventTypeSmpResponse] = ble_event_handler_smp_response,
    [BleIncomingNwpEventTypeSmpEncryptStarted] = ble_event_handler_smp_encrypt_started,
    [BleIncomingNwpEventTypeSmpLtkRequest] = ble_event_handler_smp_ltk_request,
    [BleIncomingNwpEventTypeSmpSecurityKeys] = ble_event_handler_smp_security_keys,
    [BleIncomingNwpEventTypeSmpPairingFailed] = ble_event_handler_smp_pairing_failed,
};

static inline void ble_incoming_nwp_event_processor_set_run_guard(
    BleIncomingNwpEventProcessor* instance,
    bool new_run) {
    furi_mutex_acquire(instance->lock, FuriWaitForever);
    instance->run = new_run;
    furi_mutex_release(instance->lock);
}

static void
    ble_incoming_nwp_event_processor_queue_handler(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    BleIncomingNwpEventProcessor* instance = context;
    furi_assert(instance->event_queue == object);

    BleIncomingNwpEvent* event = NULL;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        furi_assert(event->type > BleIncomingNwpEventTypeUnknown);
        furi_assert(event->type < BleIncomingNwpEventTypeCount);

        BleWorkerEventHandler handler = event_handlers[event->type];
        if(!handler(event->data_size, event->data, instance->context)) {
            BLE_LOG_W("Failed event: %d, sz: %d", event->type, event->data_size);
        }

        ble_incoming_nwp_event_free(event);
    }
}

void ble_incoming_nwp_event_processor_spawn_event(
    BleIncomingNwpEventProcessor* instance,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data) {
    furi_assert(instance);

    furi_mutex_acquire(instance->lock, FuriWaitForever);

    if(instance->run) {
        BleIncomingNwpEvent* event = ble_incoming_nwp_event_alloc(type, data_size, data);
        furi_assert(furi_message_queue_put(instance->event_queue, &event, 100) == FuriStatusOk);
    } else {
        BLE_LOG_W("Processor stopped, discard event: %d", type);
    }

    furi_mutex_release(instance->lock);
}

BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc(void* context) {
    furi_assert(context);
    BleIncomingNwpEventProcessor* instance = malloc(sizeof(BleIncomingNwpEventProcessor));
    instance->event_queue = furi_message_queue_alloc(20, sizeof(BleIncomingNwpEvent*));
    instance->lock = furi_mutex_alloc(FuriMutexTypeNormal);
    instance->context = context;

    return instance;
}

static void
    ble_incoming_nwp_event_processor_flush_pending(BleIncomingNwpEventProcessor* instance) {
    BleIncomingNwpEvent* event = NULL;

    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        ble_incoming_nwp_event_free(event);
    }
}

void ble_incoming_nwp_event_processor_subscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop) {
    furi_assert(instance);
    furi_assert(event_loop);

    instance->event_loop = event_loop;
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        ble_incoming_nwp_event_processor_queue_handler,
        instance);

    ble_incoming_nwp_event_processor_set_run_guard(instance, true);
}

void ble_incoming_nwp_event_processor_unsubscribe(
    BleIncomingNwpEventProcessor* instance,
    FuriEventLoop* event_loop) {
    furi_assert(instance);
    furi_assert(event_loop);

    ble_incoming_nwp_event_processor_set_run_guard(instance, false);
    ble_incoming_nwp_event_processor_flush_pending(instance);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
}
