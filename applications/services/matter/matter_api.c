#include "matter_i.h"

#define MATTER_API_TIMEOUT_MS (5000)

static MatterStatus
    matter_api_send_message_async(Matter* instance, const MatterApiMessage* api_message) {
    MatterStatus status = MatterStatusOk;

    const FuriStatus queue_status = furi_message_queue_put(
        instance->api_queue, api_message, furi_ms_to_ticks(MATTER_API_TIMEOUT_MS));

    if(queue_status != FuriStatusOk) {
        furi_check(queue_status == FuriStatusErrorTimeout);
        status = MatterStatusTimeout;
    }

    return status;
}

static MatterStatus matter_api_send_message(Matter* instance, MatterApiMessage* api_message) {
    MatterStatus status;

    api_message->status = &status;
    api_message->lock = api_lock_alloc_locked();

    status = matter_api_send_message_async(instance, api_message);

    if(status == MatterStatusOk) {
        api_lock_wait_unlock_and_free(api_message->lock);
    } else {
        api_lock_free(api_message->lock);
    }

    return status;
}

// =========  Message-based access API (private) =========

void matter_init_backend(Matter* instance) {
    const MatterApiMessage api_message = {
        .type = MatterApiMessageTypeInitBackend,
    };

    matter_api_send_message_async(instance, &api_message);
}

// =========  Message-based access API (public) =========

MatterStatus matter_set_switch_state(Matter* instance, MatterSwitchState switch_state) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchState,
        .data.set_switch_state.state = switch_state,
    };

    return matter_api_send_message_async(instance, &api_message);
}

MatterStatus matter_set_switch_startup_mode(Matter* instance, MatterSwitchStartupMode mode) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeSetSwitchStartupMode,
        .data.set_switch_startup_mode.mode = mode,
    };

    return matter_api_send_message_async(instance, &api_message);
}

MatterStatus matter_factory_reset(Matter* instance, MatterReboot reboot) {
    furi_check(instance);

    MatterApiMessage api_message = {
        .type = MatterApiMessageTypeFactoryReset,
        .data.factory_reset = {
            .reboot_mode = reboot,
        }};

    return matter_api_send_message_async(instance, &api_message);
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

MatterStatus matter_set_certification_config(Matter* instance, MatterCertificationType cert_type) {
    furi_check(instance);
    furi_check(cert_type < MatterCertificationTypeMax);

    return matter_certification_set_config(cert_type);
}

MatterStatus
    matter_get_certification_config(Matter* instance, MatterCertificationConfig* cert_info) {
    furi_check(instance);
    furi_check(cert_info);

    *cert_info = instance->cert_config;
    return MatterStatusOk;
}
