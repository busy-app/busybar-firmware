#include "device_name_i.h"

static void device_name_send_message(DeviceName* instance, DeviceNameMessage* message) {
    message->api_lock = api_lock_alloc_locked();
    furi_check(furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->api_lock);
}

void device_name_get(DeviceName* instance, FuriString* name) {
    furi_check(instance);
    furi_check(name);

    DeviceNameMessage message = {
        .type = DeviceNameMessageTypeGet,
        .data.get =
            {
                .name = name,
            },
    };
    device_name_send_message(instance, &message);
}

bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error) {
    furi_check(instance);
    furi_check(name);

    bool result = false;

    DeviceNameMessage message = {
        .type = DeviceNameMessageTypeSet,
        .data.set =
            {
                .name = name,
                .error = error,
                .result = &result,
            },
    };
    device_name_send_message(instance, &message);

    return result;
}

FuriPubSub* device_name_get_pubsub(DeviceName* instance) {
    furi_check(instance);
    return instance->pubsub;
}
