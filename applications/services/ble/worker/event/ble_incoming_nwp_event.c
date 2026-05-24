#include "ble_incoming_nwp_event.h"

BleIncomingNwpEvent*
    ble_incoming_nwp_event_alloc(BleIncomingNwpEventType type, size_t data_size, void* data) {
    furi_assert(type > BleIncomingNwpEventTypeUnknown);
    furi_assert(type < BleIncomingNwpEventTypeCount);

    BleIncomingNwpEvent* instance = malloc(sizeof(BleIncomingNwpEvent));
    instance->type = type;
    instance->data_size = data_size;

    if(data_size > 0) {
        instance->data = malloc(data_size);
        memcpy(instance->data, data, data_size);
    }

    return instance;
}

void ble_incoming_nwp_event_free(BleIncomingNwpEvent* instance) {
    furi_assert(instance);

    if(instance->data) {
        free(instance->data);
    }
    free(instance);
}
