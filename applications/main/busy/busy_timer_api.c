#include "busy_timer_i.h"

static void busy_timer_send_message(const BusyTimer* instance, BusyTimerMessage* message) {
    message->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->lock);
}

void busy_timer_set_callback(BusyTimer* instance, BusyTimerCallback callback, void* context) {
    furi_assert(instance);

    const BusyTimerCallbackInfo callback_info = {
        .callback = callback,
        .context = context,
    };

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetCallback,
        .data.callback_info = &callback_info,
    };

    busy_timer_send_message(instance, &message);
}

BusyTimerState busy_timer_get_state(const BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetState,
    };

    busy_timer_send_message(instance, &message);
    return message.data.state;
}

void busy_timer_get_config(const BusyTimer* instance, BusyTimerConfig* config) {
    furi_assert(instance);
    furi_assert(config);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetConfig,
        .data.config = config,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_set_config(const BusyTimer* instance, const BusyTimerConfig* config) {
    furi_assert(instance);
    furi_assert(config);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetConfig,
        .data.config_c = config,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_start(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeStart,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_stop(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeStop,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_add_time(BusyTimer* instance, int32_t time_mn) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeAddTime,
        .data.add_time_mn = time_mn,
    };

    busy_timer_send_message(instance, &message);
}
