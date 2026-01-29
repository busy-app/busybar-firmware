#include <intercom/intercom.h>
#include <toolbox/api_lock.h>

#include "matter.h"
#include "matter_common_i.h"
#include "matter_cd.h"

#define TAG            "MatterSrv"
#define FRAME_Q_SIZE   4
#define REQUEST_Q_SIZE 1
#define FIRST_TIMEOUT  (furi_ms_to_ticks(5000))
#define TIMEOUT        (furi_ms_to_ticks(200))

struct MatterSrv {
    FuriEventLoop* event_loop;
    FuriMessageQueue* frame_queue;
    FuriMessageQueue* request_queue;
    FuriPubSub* pubsub;
    IntercomChannel* intercom_ch;
    MatterCd cd;
    bool switch_state;
    uint8_t commissioned_fabrics;
    bool first_frame_sent;
};

// =========
// Utilities
// =========

static bool matter_pick_frame_of_type(
    MatterSrv* matter,
    MatterIntercomFrameType type,
    void* specific_frame,
    size_t specific_frame_size,
    FuriWait timeout) {
    furi_assert(matter);

    size_t get_result_by = furi_get_tick() + timeout;
    while(furi_get_tick() < get_result_by) {
        FuriWait max_wait = get_result_by - furi_get_tick();

        MatterIntercomFrame frame;
        FuriStatus status = furi_message_queue_get(matter->frame_queue, &frame, max_wait);
        if(status == FuriStatusErrorTimeout) {
            return false;
        } else {
            furi_check(!(status & FuriStatusError));
        }

        if(frame.type == type) {
            if(specific_frame)
                memcpy(specific_frame, &frame.frame_of_any_type, specific_frame_size);
            return true;
        }

        // messes up the order of events, doesn't matter in our case (yet)
        furi_check(furi_message_queue_put(matter->frame_queue, &frame, 0) == FuriStatusOk);
    }
    furi_crash();
}

// ======================
// Communication with 917
// ======================

static void matter_forward_frame_to_thread(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    MatterSrv* matter = context;
    const MatterIntercomFrame* frame = data;

    furi_check(furi_message_queue_put(matter->frame_queue, frame, 0) == FuriStatusOk);
}

static void matter_handle_frame(FuriEventLoopObject* object, void* context) {
    furi_assert(object);
    furi_assert(context);
    MatterSrv* matter = context;
    FuriMessageQueue* queue = object;
    furi_assert(queue == matter->frame_queue);

    MatterIntercomFrame frame;
    furi_check(furi_message_queue_get(queue, &frame, 0) == FuriStatusOk);

    if(frame.type == MatterIntercomFrameTypeSwitchState) {
        const MatterIntercomSwitchStateFrame* switch_state = &frame.switch_state;
        matter->switch_state = switch_state->value;
        MatterEvent event = {
            .type = MatterEventTypeSwitchState,
            .switch_state =
                {
                    .value = switch_state->value,
                },
        };
        furi_pubsub_publish(matter->pubsub, &event);

    } else if(frame.type == MatterIntercomFrameTypeCommissionStatus) {
        const MatterIntercomCommissionStatusFrame* status = &frame.commission_status;
        MatterEvent event = {
            .type = MatterEventTypeCommissioning,
            .commissioning =
                {
                    .status = status->status,
                },
        };
        furi_pubsub_publish(matter->pubsub, &event);

    } else if(frame.type == MatterIntercomFrameTypeFabricCountUpdate) {
        matter->commissioned_fabrics = frame.fabric_count.fabric_count;

    } else {
        furi_crash();
    }
}

static FURI_WARN_UNUSED bool
    matter_send_frame(MatterSrv* matter, const MatterIntercomFrame* frame) {
    FuriWait timeout = matter->first_frame_sent ? TIMEOUT : FIRST_TIMEOUT;
    matter->first_frame_sent = true;
    return intercom_tx(matter->intercom_ch, frame, sizeof(*frame), timeout) == sizeof(*frame);
}

// ==========
// Public API
// ==========

typedef enum {
    MatterApiRequestTypeGetSwitchState,
    MatterApiRequestTypeSetSwitchState,
    MatterApiRequestTypeSetSwitchStartupMode,
    MatterApiRequestTypeReset,
    MatterApiRequestTypeCommission,
    MatterApiRequestTypeGetFabricCount,
} MatterApiRequestType;

typedef struct {
    FuriApiLock lock;
    MatterApiRequestType type;
    bool success;
    union {
        struct {
            FuriString* qr_code;
            FuriString* manual_code;
            size_t window_duration;
        };
        bool switch_state;
        uint8_t fabric_count;
        MatterSwitchStartupMode startup_mode;
    };
} MatterApiRequest;

static void matter_handle_api_request(FuriEventLoopObject* object, void* context) {
    furi_assert(object);
    furi_assert(context);
    MatterSrv* matter = context;
    FuriMessageQueue* queue = object;
    furi_assert(queue == matter->request_queue);

    MatterApiRequest* request;
    furi_check(furi_message_queue_get(queue, &request, 0) == FuriStatusOk);
    furi_check(request);

    switch(request->type) {
    case MatterApiRequestTypeGetSwitchState: {
        request->switch_state = matter->switch_state;
        request->success = true;
        break;
    }

    case MatterApiRequestTypeSetSwitchState: {
        matter->switch_state = request->switch_state;

        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeSwitchState,
            .switch_state.value = request->switch_state,
        };
        request->success = matter_send_frame(matter, &frame);
        break;
    }

    case MatterApiRequestTypeSetSwitchStartupMode: {
        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeSwitchStartupMode,
            .startup.mode = request->startup_mode,
        };
        request->success = matter_send_frame(matter, &frame);
        break;
    }

    case MatterApiRequestTypeReset: {
        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeReset,
        };
        request->success = matter_send_frame(matter, &frame);
        break;
    }

    case MatterApiRequestTypeCommission: {
        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeCommission,
        };
        if(!matter_send_frame(matter, &frame)) {
            request->success = false;
            break;
        }

        furi_string_reset(request->qr_code);
        furi_string_reset(request->manual_code);
        MatterIntercomPairingCodesFrame codes_frame;
        request->success = matter_pick_frame_of_type(
            matter,
            MatterIntercomFrameTypePairingCodes,
            &codes_frame,
            sizeof(codes_frame),
            TIMEOUT);

        if(request->success) {
            FURI_LOG_I(TAG, "QR code: %s", codes_frame.qr_code);
            FURI_LOG_I(TAG, "Manual code: %s", codes_frame.manual_code);
            request->window_duration = MATTER_COMMISSION_TIME_SECONDS;
            furi_string_set_str(request->qr_code, codes_frame.qr_code);
            furi_string_set_str(request->manual_code, codes_frame.manual_code);
        } else {
            FURI_LOG_E(TAG, "Commissioning response timeout");
            request->window_duration = 0;
        }

        break;
    }

    case MatterApiRequestTypeGetFabricCount: {
        request->fabric_count = matter->commissioned_fabrics;
        request->success = true;
        break;
    }
    }

    api_lock_unlock(request->lock);
}

static FURI_WARN_UNUSED bool
    matter_synchronous_request(MatterSrv* matter, MatterApiRequest* request) {
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

bool matter_get_switch_state(MatterSrv* matter) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeGetSwitchState,
    };
    if(!matter_synchronous_request(matter, &request)) return false;
    return request.switch_state;
}

bool matter_set_switch_state(MatterSrv* matter, bool state) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeSetSwitchState,
        .switch_state = state,
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
    };
    if(!matter_synchronous_request(matter, &request)) return 0;
    return request.window_duration;
}

bool matter_is_commissioned(MatterSrv* matter) {
    furi_check(matter);
    MatterApiRequest request = {
        .type = MatterApiRequestTypeGetFabricCount,
    };
    if(!matter_synchronous_request(matter, &request)) return false;
    return request.fabric_count > 0;
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

// =============
// Service setup
// =============

static bool matter_send_cd_and_await_ready(MatterSrv* matter) {
    furi_assert(matter);

    MatterIntercomFrame cd_frame = {
        .type = MatterIntercomFrameTypeCdCertificate,
        .cd_certificate = {
            .contents_length = 0,
        }};

    matter_cd_init(&matter->cd);

    matter_cd_prepare_initialization_frame(&matter->cd, &cd_frame);
    if(!matter_send_frame(matter, &cd_frame)) return false;

    if(!matter_pick_frame_of_type(
           matter, MatterIntercomFrameTypeBackendReady, NULL, 0, FIRST_TIMEOUT))
        return false;

    return true;
}

MatterSrv* matter_srv_alloc(void) {
    MatterSrv* matter = malloc(sizeof(MatterSrv));

    matter->event_loop = furi_event_loop_alloc();

    matter->frame_queue = furi_message_queue_alloc(FRAME_Q_SIZE, sizeof(MatterIntercomFrame));
    furi_event_loop_subscribe_message_queue(
        matter->event_loop, matter->frame_queue, FuriEventLoopEventIn, matter_handle_frame, matter);

    matter->request_queue = furi_message_queue_alloc(REQUEST_Q_SIZE, sizeof(MatterApiRequest*));
    furi_event_loop_subscribe_message_queue(
        matter->event_loop,
        matter->request_queue,
        FuriEventLoopEventIn,
        matter_handle_api_request,
        matter);

    matter->pubsub = furi_pubsub_alloc();

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    matter->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdMatter, matter_forward_frame_to_thread, matter);

    if(!matter_send_cd_and_await_ready(matter)) FURI_LOG_E(TAG, "initialization timed out");

    furi_record_create(RECORD_MATTER, matter);
    return matter;
}

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* matter = matter_srv_alloc();
    furi_event_loop_run(matter->event_loop);

    return 0;
}
