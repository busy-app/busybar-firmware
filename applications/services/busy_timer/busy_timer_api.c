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

    BusyTimerState state;

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetState,
        .data.state = &state,
    };

    busy_timer_send_message(instance, &message);

    return state;
}

void busy_timer_get_time(const BusyTimer* instance, BusyTimerTime* time) {
    furi_assert(instance);
    furi_assert(time);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetTime,
        .data.time = time,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_get_cycles(const BusyTimer* instance, BusyTimerCycles* cycles) {
    furi_assert(instance);
    furi_assert(cycles);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetCycles,
        .data.cycles = cycles,
    };

    busy_timer_send_message(instance, &message);
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

void busy_timer_toggle(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeToggle,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_skip(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSkip,
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

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetSnapshot,
        .data.snapshot = snapshot,
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetSnapshot,
        .data.snapshot_c = snapshot,
    };

    busy_timer_send_message(instance, &message);
}
