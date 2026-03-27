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

typedef bool (*MatterResponseHandler)(MatterSrv* instance, const MatterIntercomFrame* response);

static const MatterResponseHandler matter_response_handlers[MatterIntercomFrameTypeMax];

static void matter_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    MatterSrv* matter = context;

    const FuriStatus status = furi_message_queue_put(matter->rx_queue, data, 0);

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorResource);
        FURI_LOG_W(TAG, "Dropping frame");
    }
}

static void matter_rx_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    MatterSrv* instance = context;
    furi_assert(object == instance->rx_queue);

    MatterIntercomFrame frame;
    furi_check(furi_message_queue_get(instance->rx_queue, &frame, 0) == FuriStatusOk);

    const MatterIntercomFrameType frame_type = frame.type;
    furi_assert(frame_type < MatterIntercomFrameTypeMax);

    bool should_unlock_api = false;
    const MatterResponseHandler response_handler = matter_response_handlers[frame_type];

    if(response_handler) {
        should_unlock_api = response_handler(instance, &frame);
    }

    if(should_unlock_api) {
        matter_api_unlock(instance, MatterStatusOk);
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

    if(status < MatterStatusMax) {
        matter_api_unlock(instance, status);
    } else {
        // TODO: start request timeout timer
    }
}

static void matter_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    MatterSrv* instance = context;

    if(events & MatterCustomEventRequest) {
        matter_handle_api_message(instance);
    }
}

static MatterSrv* matter_alloc(void) {
    MatterSrv* instance = malloc(sizeof(MatterSrv));

    instance->event_loop = furi_event_loop_alloc();

    instance->api_semaphore = furi_semaphore_alloc(1, 0);
    instance->rx_queue = furi_message_queue_alloc(FRAME_QUEUE_SIZE, sizeof(MatterIntercomFrame));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->rx_queue,
        FuriEventLoopEventIn,
        matter_rx_queue_callback,
        instance);

    instance->pubsub = furi_pubsub_alloc();
    instance->switch_state = furi_state_alloc(sizeof(MatterSwitchState));

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, matter_custom_event_callback, instance);

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        intercom, IntercomChannelIdMatter, matter_intercom_rx_callback, instance);

    matter_init(instance);

    furi_record_create(RECORD_MATTER, instance);

    return instance;
}

// ========= Backend response handlers  =========

static bool matter_backend_ready_response_handler(
    MatterSrv* instance,
    const MatterIntercomFrame* response) {
    UNUSED(response);
    const bool should_unlock_api =
        matter_api_is_waiting_for_response(instance, MatterApiMessageTypeInit);
    return should_unlock_api;
}

static bool
    matter_switch_state_response_handler(MatterSrv* instance, const MatterIntercomFrame* response) {
    const MatterIntercomSwitchStateFrame* switch_state_frame = &response->switch_state;
    const MatterSwitchState new_switch_state = switch_state_frame->state;

    MatterSwitchState prev_switch_state;
    furi_state_get(instance->switch_state, &prev_switch_state);

    if(new_switch_state != prev_switch_state) {
        furi_state_set(instance->switch_state, &new_switch_state);
    }

    return false;
}

static bool matter_pairing_codes_response_handler(
    MatterSrv* instance,
    const MatterIntercomFrame* response) {
    bool should_unlock_api = false;

    if(matter_api_is_waiting_for_response(instance, MatterApiMessageTypeStartCommissioning)) {
        const MatterIntercomPairingCodesFrame* pairing_codes_frame = &response->codes;
        MatterCommissioningInfo* info = instance->api_message.data.start_commissioning.info;

        FURI_LOG_I(TAG, "QR code: %s", pairing_codes_frame->qr_code);
        FURI_LOG_I(TAG, "Manual code: %s", pairing_codes_frame->manual_code);

        strlcpy(info->qr_code, pairing_codes_frame->qr_code, sizeof(info->qr_code));
        strlcpy(info->manual_code, pairing_codes_frame->manual_code, sizeof(info->manual_code));
        info->window_duration_s = MATTER_COMMISSION_TIME_SECONDS;

        should_unlock_api = true;
    }

    return should_unlock_api;
}

static bool matter_commissioning_status_response_handler(
    MatterSrv* instance,
    const MatterIntercomFrame* response) {
    const MatterIntercomCommissionStatusFrame* commission_status = &response->commission_status;

    instance->fabrics.last_status = commission_status->status;
    instance->fabrics.last_status_at = furi_hal_rtc_get_timestamp_ms();

    MatterEvent event = {
        .type = MatterEventTypeCommissioning,
        .commissioning =
            {
                .status = commission_status->status,
            },
    };

    furi_pubsub_publish(instance->pubsub, &event);

    return false;
}

static bool
    matter_fabric_count_response_handler(MatterSrv* instance, const MatterIntercomFrame* response) {
    const MatterIntercomFabricCountUpdateFrame* fabric_count = &response->fabric_count;
    instance->fabrics.count = fabric_count->fabric_count;

    return false;
}

static const MatterResponseHandler matter_response_handlers[MatterIntercomFrameTypeMax] = {
    [MatterIntercomFrameTypeBackendReady] = matter_backend_ready_response_handler,
    [MatterIntercomFrameTypeSwitchState] = matter_switch_state_response_handler,
    [MatterIntercomFrameTypePairingCodes] = matter_pairing_codes_response_handler,
    [MatterIntercomFrameTypeCommissionStatus] = matter_commissioning_status_response_handler,
    [MatterIntercomFrameTypeFabricCountUpdate] = matter_fabric_count_response_handler,
    // All other values are NULL
};

// ========= API message handlers  =========

static MatterStatus
    matter_init_api_message_handler(MatterSrv* instance, MatterApiMessageData* data) {
    UNUSED(data);

    MatterCd* cd = &instance->cd;
    matter_cd_init(cd);

    MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeInitialization,
    };

    MatterIntercomInitializationFrame* init = &frame.initialization;
    matter_cd_prepare_initialization_frame(cd, init);

    init->hardware_version_num = furi_hal_version_get_hw_version();
    strlcpy(
        init->hardware_version_str,
        furi_hal_version_get_hw_version_code(),
        sizeof(init->hardware_version_str));

    // WARNING: this if block is a temporary solution just to pass certification testing,
    // because our test lab doesn't have samples with provisioned OTP.
    // TODO: remove this if block and associated defines after we pass certification testing.
    if(!init->hardware_version_num) {
        init->hardware_version_num = DEFAULT_HARDWARE_VERSION;
        strlcpy(
            init->hardware_version_str,
            DEFAULT_HARDWARE_VERSION_STRING,
            sizeof(init->hardware_version_str));
    }

    return matter_send_frame(instance, &frame);
}

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
    UNUSED(data);
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeCommission,
    };

    return matter_send_frame(instance, &frame);
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
    [MatterApiMessageTypeInit] = matter_init_api_message_handler,
    [MatterApiMessageTypeSetSwitchState] = matter_set_switch_state_api_message_nandler,
    [MatterApiMessageTypeSetSwitchStartupMode] =
        matter_set_switch_startup_mode_api_message_nandler,
    [MatterApiMessageTypeStartCommissioning] = matter_start_commissioning_api_message_nandler,
    [MatterApiMessageTypeGetFabrics] = matter_get_commissioned_fabrics_api_message_nandler,
    [MatterApiMessageTypeFactoryReset] = matter_factory_reset_api_message_nandler,
};

// ========= Service thread  =========

int matter_srv(void* arg) {
    UNUSED(arg);

    MatterSrv* matter = matter_alloc();
    furi_event_loop_run(matter->event_loop);

    return 0;
}
