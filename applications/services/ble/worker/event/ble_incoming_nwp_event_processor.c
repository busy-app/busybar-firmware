#include "ble_incoming_nwp_event_processor.h"
#include "../../ble_common.h"

#define TAG "BleEvent"

static BleWorkerEvent*
    ble_worker_event_alloc(BleIncomingNwpEventType type, size_t data_size, void* data) {
    furi_assert(type > BleIncomingNwpEventTypeUnknown);
    furi_assert(type < BleIncomingNwpEventTypeCount);

    BleWorkerEvent* instance = malloc(sizeof(BleWorkerEvent));
    instance->type = type;
    instance->data_size = data_size;

    if(data_size > 0) {
        instance->data = malloc(data_size);
        memcpy(instance->data, data, data_size);
    }

    return instance;
}

static void ble_worker_event_free(BleWorkerEvent* instance) {
    furi_assert(instance);

    if(instance->data) {
        free(instance->data);
    }
    free(instance);
}

void ble_worker_spawn_event(
    BleEventQueuePtr queue,
    BleIncomingNwpEventType type,
    size_t data_size,
    void* data) {
    furi_assert(queue);

    BleWorkerEvent* event = ble_worker_event_alloc(type, data_size, data);
    furi_assert(furi_message_queue_put(queue, &event, 100) == FuriStatusOk);
}

void ble_worker_process_event(
    BleEventQueuePtr queue,
    const BleWorkerEventHandler* const event_handlers,
    void* context) {
    furi_assert(event_handlers);
    furi_assert(context);

    BleWorkerEvent* event = NULL;
    while(furi_message_queue_get(queue, &event, 0) == FuriStatusOk) {
        furi_assert(event->type > BleIncomingNwpEventTypeUnknown);
        furi_assert(event->type < BleIncomingNwpEventTypeCount);

        BleWorkerEventHandler handler = event_handlers[event->type];
        if(!handler(event->data_size, event->data, context)) {
            BLE_LOG_W("Failed event: %d, sz: %d", event->type, event->data_size);
        }

        ble_worker_event_free(event);
    }
}

void ble_worker_flush_events(BleEventQueuePtr queue) {
    BleWorkerEvent* event = NULL;
    while(furi_message_queue_get(queue, &event, 250) == FuriStatusOk) {
        ble_worker_event_free(event);
    }
}
