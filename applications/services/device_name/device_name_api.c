#include "device_name_i.h"

static void device_name_send_message(DeviceName* instance, DeviceNameMessage* message) {
    message->api_lock = api_lock_alloc_locked();
    furi_check(furi_message_queue_put(instance->queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->api_lock);
}

void device_name_get(DeviceName* instance, FuriString* name) {
    furi_check(instance);
    furi_check(name);

    DeviceNameMessage message = {
        .type = DeviceNameMessageTypeGetName,
        .data.get_name =
            {
                .name = name,
            },
    };
    device_name_send_message(instance, &message);
}

DeviceNameError device_name_set(DeviceName* instance, const FuriString* name) {
    furi_check(instance);
    furi_check(name);

    DeviceNameError error = DeviceNameErrorNone;

    DeviceNameMessage message = {
        .type = DeviceNameMessageTypeSetName,
        .data.set_name =
            {
                .name = name,
                .error = &error,
            },
    };
    device_name_send_message(instance, &message);

    return error;
}

FuriPubSub* device_name_get_pubsub(DeviceName* instance) {
    furi_check(instance);
    return instance->pubsub;
}
