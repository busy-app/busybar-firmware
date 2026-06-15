#include "busy_i.h"

static void busy_api_send_message_async(BusyApp* instance, const BusyApiMessage* message) {
    furi_check(
        furi_message_queue_put(instance->api_queue, message, FuriWaitForever) == FuriStatusOk);
}

static void busy_api_send_message(BusyApp* instance, BusyApiMessage* message) {
    message->lock = api_lock_alloc_locked();

    busy_api_send_message_async(instance, message);

    api_lock_wait_unlock_and_free(message->lock);
}

void busy_set_config(BusyApp* instance, const BusyAppConfig* config) {
    furi_check(instance);
    furi_check(config);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeSetConfig,
        .data.set_config.config = config,
    };

    busy_api_send_message(instance, &message);
}

void busy_show_timer(BusyApp* instance) {
    furi_check(instance);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeShowTimer,
    };

    busy_api_send_message(instance, &message);
}

void busy_request_exit(BusyApp* instance) {
    furi_check(instance);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeRequestExit,
    };

    busy_api_send_message_async(instance, &message);
}
