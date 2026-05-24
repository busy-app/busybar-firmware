#include "ble_incoming_nwp_event_processor.h"
#include "ble_incoming_nwp_event.h"

#include "_nwp_callbacks/ble_nwp_core_callbacks.h"
#include "gap/ble_worker_gap_events.h"
#include "gatt/ble_worker_gatt_events.h"
#include "smp/ble_worker_smp_events.h"

#include "../../ble_common.h"

#define TAG "BleEvent"

struct BleIncomingNwpEventProcessor {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    void* context;
};

typedef bool (*BleWorkerEventHandler)(size_t data_size, void* data, void* context);

bool ble_worker_event_handler_dummy(size_t data_size, void* data, void* context) {
    UNUSED(data_size);
    UNUSED(data);
    UNUSED(context);
    furi_crash("Unknown event!");
}

static const BleWorkerEventHandler event_handlers[BleIncomingNwpEventTypeCount] = {
    [BleIncomingNwpEventTypeUnknown] = ble_worker_event_handler_dummy,
    [BleIncomingNwpEventTypeExit] = ble_worker_event_handler_exit,
    [BleIncomingNwpEventTypeAdvReport] = ble_worker_event_handler_advertise_report,
    [BleIncomingNwpEventTypeConnected] = ble_worker_event_handler_connected,
    [BleIncomingNwpEventTypeDisconnected] = ble_worker_event_handler_disconnected,
    [BleIncomingNwpEventTypePhyUpdateComplete] = ble_worker_event_handler_phy_update_complete,
    [BleIncomingNwpEventTypeConnUpdate] = ble_worker_event_handler_connection_update,
    [BleIncomingNwpEventTypeDataLengthChange] = ble_worker_event_handler_length_change,

    [BleIncomingNwpEventTypeReceiveRemoteFeatures] =
        ble_worker_event_handler_receive_remote_features,
    [BleIncomingNwpEventTypeMoreDataRequest] = ble_worker_event_handler_more_data_request,

    [BleIncomingNwpEventTypeWrite] = ble_worker_event_handler_write_event,
    [BleIncomingNwpEventTypeDataTransmit] = ble_worker_event_handler_dummy,
    [BleIncomingNwpEventTypeMtu] = ble_worker_event_handler_mtu,
    [BleIncomingNwpEventTypeIndicateConfirm] = ble_worker_event_handler_indicate_confirm,

    [BleIncomingNwpEventTypeSmpResponse] = ble_worker_event_handler_smp_response,
    [BleIncomingNwpEventTypeSmpEncryptStarted] = ble_worker_event_handler_smp_encrypt_started,
    [BleIncomingNwpEventTypeSmpLtkRequest] = ble_worker_event_handler_smp_ltk_request,
    [BleIncomingNwpEventTypeSmpSecurityKeys] = ble_worker_event_handler_smp_security_keys,
    [BleIncomingNwpEventTypeSmpPairingFailed] = ble_worker_event_handler_smp_pairing_failed,
    [BleIncomingNwpEventTypeAdjustConnectionRequest] =
        ble_worker_event_handler_adjust_connection_request,
};

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
        if(!handler(event->data_size, event->data, context)) {
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

    BleIncomingNwpEvent* event = ble_incoming_nwp_event_alloc(type, data_size, data);
    furi_assert(furi_message_queue_put(instance->event_queue, &event, 100) == FuriStatusOk);
}

BleIncomingNwpEventProcessor* ble_incoming_nwp_event_processor_alloc() {
    BleIncomingNwpEventProcessor* instance = malloc(sizeof(BleIncomingNwpEventProcessor));
    instance->event_queue = furi_message_queue_alloc(20, sizeof(BleIncomingNwpEvent*));

    return instance;
}

void ble_incoming_nwp_event_processor_run(
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

    furi_event_loop_run(instance->event_loop);

    // ble_worker_flush_events(instance->event_queue);
    BLE_LOG_W("ble_worker_flush_events - not implemented!!!");

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
}
