#include "matter_i.h"

#include <furi_hal_rtc.h>
#include <furi_hal_version.h>
#include <power/power_service/power.h>

#define TAG "Matter"

#define RX_QUEUE_SIZE 4

#define REQUEST_TIMEOUT_MS  (furi_ms_to_ticks(5000))
#define RESPONSE_TIMEOUT_MS (5000)
#define REBOOT_TIMER_MS     (2500)

#define DEFAULT_HARDWARE_VERSION        4
#define DEFAULT_HARDWARE_VERSION_STRING "4.F22.B7.C2"

// Internal extension of MatterStatus enum
typedef enum {
    // A request needs a response from the backend
    MatterStatusExWaitForResponse = MatterStatusMax,
    MatterStatusExMax,
} MatterStatusEx;

typedef MatterStatus (*MatterApiMessageHandler)(Matter* instance, MatterApiMessageData* data);
static const MatterApiMessageHandler matter_api_message_handlers[MatterApiMessageTypeMax];

typedef void (*MatterResponseHandler)(Matter* instance, const MatterIntercomFrame* response);
static const MatterResponseHandler matter_response_handlers[MatterIntercomFrameTypeMax];

static MatterStatus matter_get_error_status_or_wait_for_response(MatterStatus status) {
    MatterStatus new_status;

    if(status == MatterStatusOk) {
        new_status = (MatterStatus)MatterStatusExWaitForResponse;
    } else {
        new_status = status;
    }

    return new_status;
}

static void matter_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    Matter* instance = context;
    const IntercomStatus intercom_status = *(IntercomStatus*)item;

    if(intercom_status == IntercomStatusOk) {
        matter_init_backend(instance);
    }
}

static void matter_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_check(data);
    furi_check(data_size == sizeof(MatterIntercomFrame));
    furi_check(context);
    Matter* instance = context;

    const FuriStatus status = furi_message_queue_put(instance->rx_queue, data, 0);

    if(status != FuriStatusOk) {
        furi_check(status == FuriStatusErrorResource);
        FURI_LOG_W(TAG, "Dropping frame");
    }
}

static void matter_rx_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    Matter* instance = context;
    furi_assert(object == instance->rx_queue);

    MatterIntercomFrame frame;
    furi_check(furi_message_queue_get(instance->rx_queue, &frame, 0) == FuriStatusOk);

    const MatterIntercomFrameType frame_type = frame.type;
    furi_assert(frame_type < MatterIntercomFrameTypeMax);

    const MatterResponseHandler response_handler = matter_response_handlers[frame_type];
    if(response_handler) response_handler(instance, &frame);
}

static void matter_timeout_timer_callback(void* context) {
    furi_assert(context);

    Matter* instance = context;
    matter_api_unlock(instance, MatterStatusTimeout);

    FURI_LOG_E(TAG, "Response timeout");
}

static MatterStatus matter_send_frame(Matter* instance, const MatterIntercomFrame* frame) {
    const size_t tx_size =
        intercom_tx(instance->intercom_ch, frame, sizeof(MatterIntercomFrame), REQUEST_TIMEOUT_MS);
    return (tx_size == sizeof(MatterIntercomFrame)) ? MatterStatusOk : MatterStatusTimeout;
}

static void matter_handle_api_message(Matter* instance) {
    MatterApiMessage* api_message = &instance->api_message;

    const MatterApiMessageType message_type = api_message->type;
    furi_assert(message_type < MatterApiMessageTypeMax);

    const MatterStatus status =
        matter_api_message_handlers[message_type](instance, &api_message->data);

    if(status == (MatterStatus)MatterStatusExWaitForResponse) {
        furi_event_loop_timer_start(instance->timeout_timer, RESPONSE_TIMEOUT_MS);
    } else {
        matter_api_unlock(instance, status);
    }
}

static void matter_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    Matter* instance = context;

    if(events & MatterCustomEventRequest) {
        matter_handle_api_message(instance);
    }
}

static void matter_switch_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    const MatterSwitchState switch_state = *(MatterSwitchState*)item;
    if(switch_state != MatterSwitchStateUnknown) {
        FURI_LOG_I(TAG, "Switch state: %s", (switch_state == MatterSwitchStateOn) ? "ON" : "OFF");
    }
}

static Matter* matter_alloc(void) {
    Matter* instance = malloc(sizeof(Matter));

    instance->event_loop = furi_event_loop_alloc();
    instance->timeout_timer = furi_event_loop_timer_alloc(
        instance->event_loop, matter_timeout_timer_callback, FuriEventLoopTimerTypeOnce, instance);
    instance->api_semaphore = furi_semaphore_alloc(1, 0);
    instance->rx_queue = furi_message_queue_alloc(RX_QUEUE_SIZE, sizeof(MatterIntercomFrame));
    instance->pubsub = furi_pubsub_alloc();
    instance->switch_state = furi_state_alloc(sizeof(MatterSwitchState));
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->rx_queue,
        FuriEventLoopEventIn,
        matter_rx_queue_callback,
        instance);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, matter_custom_event_callback, instance);

    furi_state_subscribe(instance->switch_state, matter_switch_state_callback, instance);

    if(matter_certification_read_config(&instance->cert_config) != MatterStatusOk) {
        // TODO: Can we proceed afterwards?
        FURI_LOG_E(TAG, "Failed to load certification config");
    }

    furi_state_subscribe(
        intercom_get_state(instance->intercom), matter_intercom_state_callback, instance);

    furi_record_create(RECORD_MATTER, instance);

    return instance;
}

// ========= Backend response handlers  =========

static void
    matter_backend_ready_response_handler(Matter* instance, const MatterIntercomFrame* response) {
    UNUSED(response);
    const bool should_unlock_api =
        matter_api_is_waiting_for_response(instance, MatterApiMessageTypeInitBackend);
    if(should_unlock_api) matter_api_unlock_and_cancel_timeout(instance, MatterStatusOk);
}

static void
    matter_switch_state_response_handler(Matter* instance, const MatterIntercomFrame* response) {
    const MatterIntercomSwitchStateFrame* switch_state_frame = &response->switch_state;
    const MatterSwitchState new_switch_state = switch_state_frame->state;

    MatterSwitchState prev_switch_state;
    furi_state_get(instance->switch_state, &prev_switch_state);

    if(new_switch_state != prev_switch_state) {
        furi_state_set(instance->switch_state, &new_switch_state);
    }
}

static void
    matter_pairing_codes_response_handler(Matter* instance, const MatterIntercomFrame* response) {
    if(matter_api_is_waiting_for_response(instance, MatterApiMessageTypeStartCommissioning)) {
        const MatterIntercomPairingCodesFrame* pairing_codes_frame = &response->codes;
        MatterCommissioningInfo* info = instance->api_message.data.start_commissioning.info;

        strlcpy(info->qr_code, pairing_codes_frame->qr_code, sizeof(info->qr_code));
        strlcpy(info->manual_code, pairing_codes_frame->manual_code, sizeof(info->manual_code));
        info->window_duration_s = MATTER_COMMISSION_TIME_SECONDS;

        FURI_LOG_I(TAG, "QR code: %s", info->qr_code);
        FURI_LOG_I(TAG, "Manual code: %s", info->manual_code);

        matter_api_unlock_and_cancel_timeout(
            instance, pairing_codes_frame->success ? MatterStatusOk : MatterStatusUnprovisioned);
    }
}

static void matter_commissioning_status_response_handler(
    Matter* instance,
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
}

static void
    matter_fabric_count_response_handler(Matter* instance, const MatterIntercomFrame* response) {
    const MatterIntercomFabricCountUpdateFrame* fabric_count = &response->fabric_count;
    instance->fabrics.count = fabric_count->fabric_count;
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
    matter_init_backend_api_message_handler(Matter* instance, MatterApiMessageData* data) {
    UNUSED(data);
    MatterStatus status;

    instance->intercom_ch = intercom_channel_open(
        instance->intercom, IntercomChannelIdMatter, matter_intercom_rx_callback, instance);

    do {
        MatterIntercomFrame frame = {};
        frame.type = MatterIntercomFrameTypeInitialization;

        MatterIntercomInitializationFrame* init_frame = &frame.initialization;
        const MatterCertificationConfig cert_config = instance->cert_config;

        if(cert_config.actual >= MatterCertificationTypeMax) {
            status = MatterStatusBadConfig;
            break;
        }

        status = matter_certification_get_cd(cert_config.actual, &init_frame->cd);

        if(status != MatterStatusOk) {
            break;
        }

        init_frame->hardware_version_num = furi_hal_version_get_hw_version();
        strlcpy(
            init_frame->hardware_version_str,
            furi_hal_version_get_hw_version_code(),
            sizeof(init_frame->hardware_version_str));

        // WARNING: this if block is a temporary solution just to pass certification testing,
        // because our test lab doesn't have samples with provisioned OTP.
        // TODO: remove this if block and associated defines after we pass certification testing.
        if(init_frame->hardware_version_num == 0) {
            init_frame->hardware_version_num = DEFAULT_HARDWARE_VERSION;
            strlcpy(
                init_frame->hardware_version_str,
                DEFAULT_HARDWARE_VERSION_STRING,
                sizeof(init_frame->hardware_version_str));
        }

        status = matter_send_frame(instance, &frame);

    } while(false);

    return matter_get_error_status_or_wait_for_response(status);
}

static MatterStatus
    matter_set_switch_state_api_message_handler(Matter* instance, MatterApiMessageData* data) {
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

static MatterStatus matter_set_switch_startup_mode_api_message_handler(
    Matter* instance,
    MatterApiMessageData* data) {
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeSwitchStartupMode,
        .startup.mode = data->set_switch_startup_mode.mode,
    };

    return matter_send_frame(instance, &frame);
}

static MatterStatus
    matter_start_commissioning_api_message_handler(Matter* instance, MatterApiMessageData* data) {
    UNUSED(data);
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeCommission,
    };

    const MatterStatus status = matter_send_frame(instance, &frame);
    return matter_get_error_status_or_wait_for_response(status);
}

static MatterStatus matter_get_commissioned_fabrics_api_message_handler(
    Matter* instance,
    MatterApiMessageData* data) {
    *data->get_fabrics.fabrics = instance->fabrics;
    return MatterStatusOk;
}

static void matter_deferred_reboot(void* context) {
    UNUSED(context);

    Power* power = furi_record_open(RECORD_POWER);
    power_reboot(power, PowerRebootNormal);
    while(1)
        ;
}

static MatterStatus
    matter_factory_reset_api_message_handler(Matter* instance, MatterApiMessageData* data) {
    UNUSED(data);
    const MatterIntercomFrame frame = {
        .type = MatterIntercomFrameTypeReset,
    };

    MatterStatus status = matter_send_frame(instance, &frame);
    if(status != MatterStatusOk) return status;

    if(data->factory_reset.reboot_mode == MatterRebootAutomatically) {
        MatterEvent event = {
            .type = MatterEventTypeWillReboot,
        };
        furi_pubsub_publish(instance->pubsub, &event);

        // this object leaks
        // it's fine - it'll be freed by the ultimate garbage collector! a reset!
        FuriTimer* timer = furi_timer_alloc(matter_deferred_reboot, FuriTimerTypeOnce, NULL);
        furi_timer_start(timer, furi_ms_to_ticks(REBOOT_TIMER_MS));
    }

    return MatterStatusOk;
}

static const MatterApiMessageHandler matter_api_message_handlers[MatterApiMessageTypeMax] = {
    [MatterApiMessageTypeInitBackend] = matter_init_backend_api_message_handler,
    [MatterApiMessageTypeSetSwitchState] = matter_set_switch_state_api_message_handler,
    [MatterApiMessageTypeSetSwitchStartupMode] =
        matter_set_switch_startup_mode_api_message_handler,
    [MatterApiMessageTypeStartCommissioning] = matter_start_commissioning_api_message_handler,
    [MatterApiMessageTypeGetFabrics] = matter_get_commissioned_fabrics_api_message_handler,
    [MatterApiMessageTypeFactoryReset] = matter_factory_reset_api_message_handler,
};

// ========= Service thread  =========

int matter_srv(void* arg) {
    UNUSED(arg);

    Matter* matter = matter_alloc();
    furi_event_loop_run(matter->event_loop);

    return 0;
}
