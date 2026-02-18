#include "busy_timer_i.h"

static void busy_timer_send_message(const BusyTimer* instance, BusyTimerMessage* message) {
    message->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->lock);
}

void busy_timer_get_run_info(const BusyTimer* instance, BusyTimerRunInfo* info) {
    furi_assert(instance);
    furi_assert(info);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetRunInfo,
        .data.get_run_info =
            {
                .run_info = info,
            },
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

void busy_timer_add_time(BusyTimer* instance, int32_t time_minutes) {
    furi_assert(instance);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeAddTime,
        .data.add_time =
            {
                .time_minutes = time_minutes,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetSnapshot,
        .data.set_snapshot =
            {
                .snapshot = snapshot,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_set_snapshot(BusyTimer* instance, const BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetSnapshot,
        .data.set_snapshot =
            {
                .snapshot = snapshot,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_get_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerProfile* profile) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(profile);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetProfile,
        .data.get_profile =
            {
                .profile_id = profile_id,
                .profile = profile,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_set_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerProfile* profile) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(profile);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetProfile,
        .data.set_profile =
            {
                .profile_id = profile_id,
                .profile = profile,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_load_profile(BusyTimer* instance, BusyTimerProfileId profile_id) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeLoadProfile,
        .data.load_profile =
            {
                .profile_id = profile_id,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_get_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerPreset* preset) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(preset);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeGetPreset,
        .data.get_preset =
            {
                .profile_id = profile_id,
                .preset = preset,
            },
    };

    busy_timer_send_message(instance, &message);
}

void busy_timer_set_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerPreset* preset) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(preset);

    BusyTimerMessage message = {
        .type = BusyTimerMessageTypeSetPreset,
        .data.set_preset =
            {
                .profile_id = profile_id,
                .preset = preset,
            },
    };

    busy_timer_send_message(instance, &message);
}
