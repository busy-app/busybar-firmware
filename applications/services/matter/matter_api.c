#include "matter_i.h"

static bool matter_synchronous_request(MatterSrv* matter, MatterApiRequest* request) {
    request->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(matter->request_queue, &request, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(request->lock);
    if(!request->success) {
        FURI_LOG_E(TAG, "Request failed");
    }
    return request->success;
}

FuriPubSub* matter_get_pubsub(MatterSrv* matter) {
    furi_check(matter);
    return matter->pubsub;
}

FuriState* matter_get_switch_state(MatterSrv* matter) {
    furi_check(matter);
    return matter->switch_state;
}

bool matter_set_switch_state(MatterSrv* matter, MatterSwitchState switch_state) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeSetSwitchState,
        .switch_state = switch_state,
    };
    return matter_synchronous_request(matter, &request);
}

bool matter_set_switch_startup_mode(MatterSrv* matter, MatterSwitchStartupMode mode) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeSetSwitchStartupMode,
        .startup_mode = mode,
    };
    return matter_synchronous_request(matter, &request);
}

bool matter_factory_reset(MatterSrv* matter) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeReset,
    };
    return matter_synchronous_request(matter, &request);
}

size_t
    matter_enable_commissioning(MatterSrv* matter, FuriString* qr_code, FuriString* manual_code) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeCommission,
        .qr_code = qr_code,
        .manual_code = manual_code,
        .window_duration = 0,
    };
    matter_synchronous_request(matter, &request);
    return request.window_duration;
}

MatterCommissionedFabrics matter_commissioned_fabrics(MatterSrv* matter) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeGetFabrics,
        .fabrics = {
            .count = 0,
            .last_status = MatterCommissioningStatusMAX,
            .last_status_at = 0,
        }};
    matter_synchronous_request(matter, &request);
    return request.fabrics;
}

const char* matter_get_wanted_cd_selection(MatterSrv* matter) {
    furi_assert(matter);
    return matter_cd_get_wanted_selection(&matter->cd);
}

bool matter_set_wanted_cd_selection(MatterSrv* matter, const char* selection) {
    furi_assert(matter);
    furi_assert(selection);
    return matter_cd_set_wanted_selection(&matter->cd, selection);
}

const char* matter_get_de_facto_cd_selection(MatterSrv* matter) {
    furi_assert(matter);
    return matter_cd_get_de_facto_selection(&matter->cd);
}
