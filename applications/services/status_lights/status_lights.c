#include "status_lights_i.h"

#define STATUS_LIGHTS_REQUEST_TIMEOUT_MS (5000)

#define STATUS_LIGHTS_API_SEM_COUNT      (1)
#define STATUS_LIGHTS_API_SEM_COUNT_INIT (0)

typedef StatusLightsStatus (
    *StatusLightsApiMessageHandler)(StatusLights* instance, StatusLightsApiMessage* message);

static const StatusLightsApiMessageHandler api_message_handlers[];

static StatusLightsStatus
    status_lights_send_command(StatusLights* instance, const StatusLightsCommand* command) {
    StatusLightsStatus status;

    do {
        if(instance->intercom_ch == NULL) {
            status = StatusLightsStatusError;
            break;
        }

        const size_t tx_size = intercom_tx(
            instance->intercom_ch, command, sizeof(*command), STATUS_LIGHTS_REQUEST_TIMEOUT_MS);

        if(tx_size != sizeof(*command)) {
            status = StatusLightsStatusTimeout;
            break;
        }

        status = StatusLightsStatusOk;
    } while(false);

    return status;
}

static void status_lights_init(StatusLights* instance) {
    furi_assert(instance->intercom_ch == NULL);
    // NOTE: Not expecting any messages from Intercom
    instance->intercom_ch =
        intercom_channel_open(instance->intercom, IntercomChannelIdStatusLights, NULL, NULL);

    status_lights_api_unlock(instance, StatusLightsStatusOk);
}

static void status_lights_deinit(StatusLights* instance) {
    instance->intercom_ch = NULL;

    if(status_lights_api_is_locked(instance)) {
        status_lights_api_unlock(instance, StatusLightsStatusError);
    }
}

static StatusLightsStatus
    status_lights_do_set_brightness(StatusLights* instance, StatusLightsApiMessage* message) {
    const StatusLightsApiMessageSetBrightness* set_brightness = &message->set_brightness;

    const uint8_t brightness_val = CLAMP(
        set_brightness->brightness.val,
        STATUS_LIGHTS_BRIGHTNESS_MAX,
        STATUS_LIGHTS_BRIGHTNESS_MIN);

    instance->brightness.val = brightness_val;

    const StatusLightsCommand command = {
        .id = StatusLightsCommandIdSetBrightness,
        .set_brightness =
            {
                .brightness = brightness_val,
            },
    };

    return status_lights_send_command(instance, &command);
}

static StatusLightsStatus
    status_lights_do_get_brightness(StatusLights* instance, StatusLightsApiMessage* message) {
    StatusLightsApiMessageGetBrightness* get_brightness = &message->get_brightness;

    *get_brightness->brightness = instance->brightness;

    return StatusLightsStatusOk;
}

static StatusLightsStatus
    status_lights_do_run_preset(StatusLights* instance, StatusLightsApiMessage* message) {
    const StatusLightsApiMessageRunPreset* run_preset = &message->run_preset;

    const StatusLightsCommand command = {
        .id = StatusLightsCommandIdRunPreset,
        .run_preset =
            {
                .preset = run_preset->preset,
                .color = run_preset->color,
            },
    };

    return status_lights_send_command(instance, &command);
}

static void status_lights_process_request(StatusLights* instance) {
    StatusLightsApiMessage* message = &instance->api_message;
    const StatusLightsApiMessageType message_type = message->type;
    furi_assert(message_type < StatusLightsApiMessageTypeMax);

    const StatusLightsStatus status = api_message_handlers[message_type](instance, message);
    status_lights_api_unlock(instance, status);
}

static void status_lights_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    StatusLights* instance = context;
    // NOTE: Events are mutually exclusive in the order below
    if(events & StatusLightsCustomEventDeinit) {
        status_lights_deinit(instance);
    } else if(events & StatusLightsCustomEventRequest) {
        status_lights_process_request(instance);
    } else if(events & StatusLightsCustomEventInit) {
        status_lights_init(instance);
    }
}

static void status_lights_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    StatusLights* instance = context;
    const IntercomStatus intercom_status = *(IntercomStatus*)item;

    if(intercom_status == IntercomStatusOk) {
        furi_event_loop_set_custom_event(instance->event_loop, StatusLightsCustomEventInit);
    } else if(intercom_status != IntercomStatusUnknown) {
        furi_event_loop_set_custom_event(instance->event_loop, StatusLightsCustomEventDeinit);
    }
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(*instance));

    instance->brightness = (const StatusLightsBrightness){.val = STATUS_LIGHTS_BRIGHTNESS_DEFAULT};

    instance->event_loop = furi_event_loop_alloc();
    instance->api_semaphore =
        furi_semaphore_alloc(STATUS_LIGHTS_API_SEM_COUNT, STATUS_LIGHTS_API_SEM_COUNT_INIT);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, status_lights_custom_event_callback, instance);

    furi_state_subscribe(
        intercom_get_state(instance->intercom), status_lights_intercom_state_callback, instance);

    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    return instance;
}

int32_t status_lights_srv(void* p) {
    UNUSED(p);

    StatusLights* instance = status_lights_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

static const StatusLightsApiMessageHandler api_message_handlers[] = {
    [StatusLightsApiMessageTypeSetBrightness] = status_lights_do_set_brightness,
    [StatusLightsApiMessageTypeGetBrightness] = status_lights_do_get_brightness,
    [StatusLightsApiMessageTypeRunPreset] = status_lights_do_run_preset,
};

static_assert(COUNT_OF(api_message_handlers) == StatusLightsApiMessageTypeMax);
