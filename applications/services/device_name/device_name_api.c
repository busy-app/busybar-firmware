#include "device_name_i.h"

void device_name_get(DeviceName* instance, FuriString* name) {
    furi_check(instance);
    furi_check(name);

    DeviceNameMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = DeviceNameMessageTypeGet,
        .data.get =
            {
                .name = name,
            },
    };

    furi_check(furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.api_lock);
}

bool device_name_set(DeviceName* instance, FuriString* name, FuriString* error) {
    furi_check(instance);
    furi_check(name);

    bool result = false;

    DeviceNameMessage message = {
        .api_lock = api_lock_alloc_locked(),
        .type = DeviceNameMessageTypeSet,
        .data.set =
            {
                .name = name,
                .error = error,
                .result = &result,
            },
    };

    furi_check(furi_message_queue_put(instance->queue, &message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result;
}

FuriPubSub* device_name_get_pubsub(DeviceName* instance) {
    furi_check(instance);
    return instance->pubsub;
}
