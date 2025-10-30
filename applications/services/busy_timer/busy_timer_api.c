#include "busy_timer_i.h"

static void busy_timer_api_send_message(BusyTimer* instance, BusyTimerApiMessage* message) {
    message->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(instance->api_queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->lock);
}

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeGetSnapshot,
        .get_snapshot =
            {
                .snapshot = snapshot,
            },
    };

    busy_timer_api_send_message(instance, &message);
}

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeSetSnapshot,
        .set_snapshot =
            {
                .snapshot = snapshot,
            },
    };

    busy_timer_api_send_message(instance, &message);
}
