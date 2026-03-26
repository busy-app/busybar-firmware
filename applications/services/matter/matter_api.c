#include "matter_i.h"

static MatterStatus matter_send_api_message(MatterSrv* instance, MatterApiMessage* api_message) {
    MatterStatus status;

    api_message->status = &status;
    api_message->lock = api_lock_alloc_locked();

    furi_check(furi_semaphore_acquire(instance->api_semaphore, FuriWaitForever) == FuriStatusOk);

    instance->api_message = *api_message;
    furi_event_loop_set_custom_event(instance->event_loop, MatterCustomEventRequest);

    api_lock_wait_unlock_and_free(api_message->lock);

    return status;
}

// =========  Message-based access API (public) =========

MatterStatus matter_set_switch_state(MatterSrv* instance, MatterSwitchState switch_state) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchState,
        .data.set_switch_state.state = switch_state,
    };

    return matter_send_api_message(instance, &api_message);
}

MatterStatus matter_set_switch_startup_mode(MatterSrv* instance, MatterSwitchStartupMode mode) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchStartupMode,
        .data.set_switch_startup_mode.mode = mode,
    };

    return matter_send_api_message(instance, &api_message);
}

MatterStatus matter_factory_reset(MatterSrv* instance) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeFactoryReset,
    };

    return matter_send_api_message(instance, &api_message);
}

MatterStatus matter_enable_commissioning(MatterSrv* instance, MatterCommissioningInfo* info) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeStartCommissioning,
        .data.start_commissioning.info = info,
    };

    return matter_send_api_message(instance, &api_message);
}

MatterStatus
    matter_get_commissioned_fabrics(MatterSrv* instance, MatterCommissionedFabrics* fabrics) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeGetFabrics,
        .data.get_fabrics.fabrics = fabrics,
    };

    return matter_send_api_message(instance, &api_message);
}

// ========= Direct access API (public) =========

FuriPubSub* matter_get_pubsub(MatterSrv* instance) {
    furi_check(instance);
    return instance->pubsub;
}

FuriState* matter_get_switch_state(MatterSrv* instance) {
    furi_check(instance);
    return instance->switch_state;
}

const char* matter_get_wanted_cd_selection(MatterSrv* instance) {
    furi_assert(instance);
    return matter_cd_get_wanted_selection(&instance->cd);
}

MatterStatus matter_set_wanted_cd_selection(MatterSrv* instance, const char* selection) {
    furi_assert(instance);
    furi_assert(selection);
    return matter_cd_set_wanted_selection(&instance->cd, selection);
}

const char* matter_get_de_facto_cd_selection(MatterSrv* instance) {
    furi_assert(instance);
    return matter_cd_get_de_facto_selection(&instance->cd);
}
