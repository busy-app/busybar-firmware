#include "matter_i.h"

static void
    matter_api_send_message_internal(Matter* instance, const MatterApiMessage* api_message) {
    instance->api_message = *api_message;
    furi_event_loop_set_custom_event(instance->event_loop, MatterCustomEventRequest);
}

static MatterStatus matter_api_send_message(Matter* instance, MatterApiMessage* api_message) {
    MatterStatus status;

    api_message->status = &status;
    api_message->lock = api_lock_alloc_locked();

    furi_check(furi_semaphore_acquire(instance->api_semaphore, FuriWaitForever) == FuriStatusOk);

    matter_api_send_message_internal(instance, api_message);
    api_lock_wait_unlock_and_free(api_message->lock);

    return status;
}

static void matter_api_reset_message(MatterApiMessage* api_message) {
    memset(api_message, 0, sizeof(MatterApiMessage));
    api_message->type = MatterApiMessageTypeMax;
}

// =========  Message-based access API (private) =========

void matter_init(Matter* instance) {
    const MatterApiMessage api_message = {
        .type = MatterApiMessageTypeInit,
    };

    matter_api_send_message_internal(instance, &api_message);
}

// =========  Message-based access API (public) =========

MatterStatus matter_set_switch_state(Matter* instance, MatterSwitchState switch_state) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchState,
        .data.set_switch_state.state = switch_state,
    };

    return matter_api_send_message(instance, &api_message);
}

MatterStatus matter_set_switch_startup_mode(Matter* instance, MatterSwitchStartupMode mode) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchStartupMode,
        .data.set_switch_startup_mode.mode = mode,
    };

    return matter_api_send_message(instance, &api_message);
}

MatterStatus matter_factory_reset(Matter* instance) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeFactoryReset,
    };

    return matter_api_send_message(instance, &api_message);
}

MatterStatus matter_enable_commissioning(Matter* instance, MatterCommissioningInfo* info) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeStartCommissioning,
        .data.start_commissioning.info = info,
    };

    return matter_api_send_message(instance, &api_message);
}

MatterStatus
    matter_get_commissioned_fabrics(Matter* instance, MatterCommissionedFabrics* fabrics) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeGetFabrics,
        .data.get_fabrics.fabrics = fabrics,
    };

    return matter_api_send_message(instance, &api_message);
}

// ========= Direct access API (public) =========

FuriPubSub* matter_get_pubsub(Matter* instance) {
    furi_check(instance);
    return instance->pubsub;
}

FuriState* matter_get_switch_state(Matter* instance) {
    furi_check(instance);
    return instance->switch_state;
}

const char* matter_get_wanted_cd_selection(Matter* instance) {
    furi_assert(instance);
    return matter_cd_get_wanted_selection(&instance->cd);
}

MatterStatus matter_set_wanted_cd_selection(Matter* instance, const char* selection) {
    furi_assert(instance);
    furi_assert(selection);
    return matter_cd_set_wanted_selection(&instance->cd, selection);
}

const char* matter_get_de_facto_cd_selection(Matter* instance) {
    furi_assert(instance);
    return matter_cd_get_de_facto_selection(&instance->cd);
}

// Internal API

bool matter_api_is_waiting_for_response(Matter* instance, MatterApiMessageType message_type) {
    return instance->api_message.type == message_type;
}

void matter_api_unlock(Matter* instance, MatterStatus status) {
    MatterApiMessage* api_message = &instance->api_message;
    furi_assert(api_message->type < MatterApiMessageTypeMax);

    if(api_message->status) {
        *api_message->status = status;
    }

    if(api_message->lock) {
        api_lock_unlock(api_message->lock);
    }

    matter_api_reset_message(api_message);

    furi_check(furi_semaphore_release(instance->api_semaphore) == FuriStatusOk);
}
