#include "busy_i.h"

static void busy_api_send_message(BusyApp* instance, BusyApiMessage* message) {
    message->lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->api_queue, message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message->lock);
}

void busy_show_timer(BusyApp* instance) {
    furi_check(instance);

    BusyApiMessage message = {
        .type = BusyApiMessageTypeShowTimer,
    };

    busy_api_send_message(instance, &message);
}
