#include "busy_timer_i.h"

static void
    busy_timer_api_blocking_request(const BusyTimer* instance, BusyTimerApiMessage* message) {
    message->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(instance->api_queue, message, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(message->lock);
}

static void
    busy_timer_api_asynchronous_request(const BusyTimer* instance, BusyTimerApiMessage* message) {
    message->lock = NULL;
    furi_check(
        furi_message_queue_put(instance->api_queue, message, FuriWaitForever) == FuriStatusOk);
}

void busy_timer_get_run_info(const BusyTimer* instance, BusyTimerRunInfo* info) {
    furi_assert(instance);
    furi_assert(info);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeGetRunInfo,
        .data.get_run_info =
            {
                .run_info = info,
            },
    };

    busy_timer_api_blocking_request(instance, &message);
}

void busy_timer_start(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerSessionSource source) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeStart,
        .data.start =
            {
                .profile_id = profile_id,
                .source = source,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_stop(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeStop,
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_toggle(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeToggle,
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_skip(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeSkip,
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_finalize(BusyTimer* instance) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeFinalize,
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_add_time(BusyTimer* instance, int32_t time_minutes) {
    furi_assert(instance);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeAddTime,
        .data.add_time =
            {
                .time_minutes = time_minutes,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_get_snapshot(BusyTimer* instance, BusyTimerSnapshot* snapshot) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeGetSnapshot,
        .data.get_snapshot =
            {
                .snapshot = snapshot,
            },
    };

    busy_timer_api_blocking_request(instance, &message);
}

void busy_timer_set_snapshot(
    BusyTimer* instance,
    const BusyTimerSnapshot* snapshot,
    BusyTimerSessionSource source) {
    furi_check(instance);
    furi_check(snapshot);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeSetSnapshot,
        .data.set_snapshot =
            {
                .snapshot = *snapshot,
                .source = source,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_get_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerProfile* profile) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(profile);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeGetProfile,
        .data.get_profile =
            {
                .profile_id = profile_id,
                .profile = profile,
            },
    };

    busy_timer_api_blocking_request(instance, &message);
}

void busy_timer_set_profile(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerProfile* profile) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(profile);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeSetProfile,
        .data.set_profile =
            {
                .profile_id = profile_id,
                .profile = *profile,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

void busy_timer_get_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    BusyTimerPreset* preset) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(preset);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeGetPreset,
        .data.get_preset =
            {
                .profile_id = profile_id,
                .preset = preset,
            },
    };

    busy_timer_api_blocking_request(instance, &message);
}

void busy_timer_set_preset(
    BusyTimer* instance,
    BusyTimerProfileId profile_id,
    const BusyTimerPreset* preset) {
    furi_check(instance);
    furi_check(profile_id < BusyTimerProfileIdMax);
    furi_check(preset);

    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeSetPreset,
        .data.set_preset =
            {
                .profile_id = profile_id,
                .preset = *preset,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}

// Private API calls

void busy_timer_handle_matter(BusyTimer* instance, MatterSwitchState switch_state) {
    BusyTimerApiMessage message = {
        .type = BusyTimerApiMessageTypeHandleMatter,
        .data.handle_matter =
            {
                .switch_state = switch_state,
            },
    };

    busy_timer_api_asynchronous_request(instance, &message);
}
