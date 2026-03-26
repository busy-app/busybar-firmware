#include "matter_i.h"

#include <furi_hal_rtc.h>
#include <furi_hal_version.h>

#define FRAME_QUEUE_SIZE 4
#define API_QUEUE_SIZE   1

#define FIRST_TIMEOUT (furi_ms_to_ticks(5000))
#define TIMEOUT       (furi_ms_to_ticks(200))

#define DEFAULT_HARDWARE_VERSION        4
#define DEFAULT_HARDWARE_VERSION_STRING "4.F22.B7.C2"

typedef MatterStatus (*MatterApiMessageHandler)(MatterSrv* instance, MatterApiMessageData* data);

static const MatterApiMessageHandler matter_api_message_handlers[MatterApiMessageTypeMax];

// =========
// Utilities
// =========

static MatterStatus matter_wait_for_response(
    MatterSrv* matter,
    MatterIntercomFrameType type,
    void* specific_frame,
    size_t specific_frame_size,
    uint32_t timeout) {
    furi_assert(matter);

    const uint32_t get_result_by = furi_get_tick() + timeout;

    while(furi_get_tick() < get_result_by) {
        const uint32_t max_wait = get_result_by - furi_get_tick();

        MatterIntercomFrame frame;
        FuriStatus status = furi_message_queue_get(matter->frame_queue, &frame, max_wait);

        if(status == FuriStatusErrorTimeout) {
            return MatterStatusTimeout;
        }

        furi_check((status & FuriStatusError) == 0);

        if(frame.type == type) {
            if(specific_frame) {
                memcpy(specific_frame, &frame, specific_frame_size);
            }

            return MatterStatusOk;
        }

        // messes up the order of events, doesn't matter in our case (yet)
        furi_check(furi_message_queue_put(matter->frame_queue, &frame, 0) == FuriStatusOk);
    }

    // Shouldn't ever get here
    furi_crash();
}

// ======================
// Communication with 917
// ======================

static void matter_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    MatterSrv* matter = context;

    const FuriStatus status = furi_message_queue_put(matter->frame_queue, data, 0);

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorResource);
        FURI_LOG_W(TAG, "Dropping frame");
    }
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
        const MatterIntercomSwitchStateFrame* switch_state_frame = &frame.switch_state;
        const MatterSwitchState new_switch_state = switch_state_frame->state;

        MatterSwitchState prev_switch_state;
        furi_state_get(matter->switch_state, &prev_switch_state);

        if(new_switch_state != prev_switch_state) {
            furi_state_set(matter->switch_state, &new_switch_state);
        }

    } else if(frame.type == MatterIntercomFrameTypeCommissionStatus) {
        const MatterIntercomCommissionStatusFrame* status = &frame.commission_status;
        matter->fabrics.last_status = status->status;
        matter->fabrics.last_status_at = furi_hal_rtc_get_timestamp_ms();
        MatterEvent event = {
            .type = MatterEventTypeCommissioning,
            .commissioning =
                {
                    .status = status->status,
                },
        };
        furi_pubsub_publish(matter->pubsub, &event);

    } else if(frame.type == MatterIntercomFrameTypeFabricCountUpdate) {
        matter->fabrics.count = frame.fabric_count.fabric_count;

    } else {
        furi_crash("Invalid MatterIntercomFrameType value");
    }
}

static MatterStatus matter_send_frame(MatterSrv* matter, const MatterIntercomFrame* frame) {
    const uint32_t timeout = matter->first_frame_sent ? TIMEOUT : FIRST_TIMEOUT;
    matter->first_frame_sent = true;

    const size_t tx_size =
        intercom_tx(matter->intercom_ch, frame, sizeof(MatterIntercomFrame), timeout);
    return (tx_size == sizeof(MatterIntercomFrame)) ? MatterStatusOk : MatterStatusTimeout;
}

static void matter_handle_api_message(MatterSrv* instance) {
    MatterApiMessage* api_message = &instance->api_message;

    const MatterApiMessageType message_type = api_message->type;
    furi_assert(message_type < MatterApiMessageTypeMax);

    const MatterStatus status =
        matter_api_message_handlers[message_type](instance, &api_message->data);
    *api_message->status = status;

    if(api_message->lock) {
        api_lock_unlock(api_message->lock);
    }

    furi_check(furi_semaphore_release(instance->api_semaphore) == FuriStatusOk);
}

static void matter_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    MatterSrv* instance = context;

    if(events & MatterCustomEventRequest) {
        matter_handle_api_message(instance);
    }
}

// =============
// Service setup
// =============

static MatterStatus matter_send_initialization_and_await_ready(MatterSrv* matter) {
    furi_assert(matter);
    matter_cd_init(&matter->cd);

    MatterIntercomFrame cd_frame;
    memset(&cd_frame, 0, sizeof(cd_frame));
    cd_frame.type = MatterIntercomFrameTypeInitialization;
    MatterIntercomInitializationFrame* init = &cd_frame.initialization;

    matter_cd_prepare_initialization_frame(&matter->cd, init);

    init->hardware_version_num = furi_hal_version_get_hw_version();
    strcpy(init->hardware_version_str, furi_hal_version_get_hw_version_code());

    // WARNING: this if block is a temporary solution just to pass certification testing,
    // because our test lab doesn't have samples with provisioned OTP.
    // TODO: remove this if block and associated defines after we pass certification testing.
    if(!init->hardware_version_num) {
        init->hardware_version_num = DEFAULT_HARDWARE_VERSION;
        strcpy(init->hardware_version_str, DEFAULT_HARDWARE_VERSION_STRING);
    }

    MatterStatus status;

    do {
        status = matter_send_frame(matter, &cd_frame);

        if(status != MatterStatusOk) {
            break;
        }

        status = matter_wait_for_response(
            matter, MatterIntercomFrameTypeBackendReady, NULL, 0, FIRST_TIMEOUT);

    } while(false);

    return status;
}

static MatterSrv* matter_alloc(void) {
    MatterSrv* instance = malloc(sizeof(MatterSrv));

    instance->event_loop = furi_event_loop_alloc();

    instance->api_semaphore = furi_semaphore_alloc(1, 0);
    instance->frame_queue =
        furi_message_queue_alloc(FRAME_QUEUE_SIZE, sizeof(MatterIntercomFrame));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->frame_queue,
        FuriEventLoopEventIn,
        matter_handle_frame,
        instance);

    instance->pubsub = furi_pubsub_alloc();
    instance->switch_state = furi_state_alloc(sizeof(MatterSwitchState));

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, matter_custom_event_callback, instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdMatter, matter_intercom_rx_callback, instance);

    if(matter_send_initialization_and_await_ready(instance) == MatterStatusOk) {
        furi_semaphore_release(instance->api_semaphore);
    } else {
        FURI_LOG_E(TAG, "initialization timed out");
    }

    furi_record_create(RECORD_MATTER, instance);
    return instance;
}

// ========= API message handlers  =========

static MatterStatus
    matter_set_switch_state_api_message_nandler(MatterSrv* instance, MatterApiMessageData* data) {
    MatterStatus status = MatterStatusOk;

    MatterSwitchState prev_switch_state;
    furi_state_get(instance->switch_state, &prev_switch_state);

    const MatterSwitchState new_switch_state = data->set_switch_state.state;

    if(new_switch_state != prev_switch_state) {
        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeSwitchState,
            .switch_state.state = new_switch_state,
        };

        status = matter_send_frame(instance, &frame);
    }

    return status;
}

static MatterStatus matter_set_switch_startup_mode_api_message_nandler(
    MatterSrv* instance,
    MatterApiMessageData* data) {
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeSwitchStartupMode,
        .startup.mode = data->set_switch_startup_mode.mode,
    };

    return matter_send_frame(instance, &frame);
}

static MatterStatus matter_start_commissioning_api_message_nandler(
    MatterSrv* instance,
    MatterApiMessageData* data) {
    MatterStatus status;
    do {
        const MatterIntercomFrame frame = {
            .type = MatterIntercomFrameTypeCommission,
        };

        status = matter_send_frame(instance, &frame);

        if(status != MatterStatusOk) {
            break;
        }

        MatterIntercomPairingCodesFrame response;

        status = matter_wait_for_response(
            instance, MatterIntercomFrameTypePairingCodes, &response, sizeof(response), TIMEOUT);

        MatterCommissioningInfo* info = data->start_commissioning.info;

        if(status == MatterStatusOk) {
            FURI_LOG_I(TAG, "QR code: %s", response.qr_code);
            FURI_LOG_I(TAG, "Manual code: %s", response.manual_code);

            strlcpy(info->qr_code, response.qr_code, sizeof(info->qr_code));
            strlcpy(info->manual_code, response.manual_code, sizeof(info->manual_code));
            info->window_duration_s = MATTER_COMMISSION_TIME_SECONDS;

        } else {
            FURI_LOG_E(TAG, "Commissioning response timeout");
        }

    } while(false);

    return status;
}

static MatterStatus matter_get_commissioned_fabrics_api_message_nandler(
    MatterSrv* instance,
    MatterApiMessageData* data) {
    *data->get_fabrics.fabrics = instance->fabrics;
    return MatterStatusOk;
}

static MatterStatus
    matter_factory_reset_api_message_nandler(MatterSrv* instance, MatterApiMessageData* data) {
    UNUSED(data);
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeReset,
    };

    return matter_send_frame(instance, &frame);
}

static const MatterApiMessageHandler matter_api_message_handlers[MatterApiMessageTypeMax] = {
    [MatterApiMessageTypeSetSwitchState] = matter_set_switch_state_api_message_nandler,
    [MatterApiMessageTypeSetSwitchStartupMode] =
        matter_set_switch_startup_mode_api_message_nandler,
    [MatterApiMessageTypeStartCommissioning] = matter_start_commissioning_api_message_nandler,
    [MatterApiMessageTypeGetFabrics] = matter_get_commissioned_fabrics_api_message_nandler,
    [MatterApiMessageTypeFactoryReset] = matter_factory_reset_api_message_nandler,
};

// Service thread

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* matter = matter_alloc();
    furi_event_loop_run(matter->event_loop);

    return 0;
}
