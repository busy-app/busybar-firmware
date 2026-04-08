#include "status_lights_i.h"

#define TAG "StatusLights"

typedef void (
    *StatusLightsApiMessageHandler)(StatusLights* instance, StatusLightsApiMessage* message);

static const StatusLightsApiMessageHandler api_message_handlers[];

static void
    status_lights_send_command(StatusLights* instance, const StatusLightsCommand* command) {
    // TODO: Better state check & return codes
    if(!instance->intercom_ch) {
        return;
    }

    size_t tx_size =
        intercom_tx(instance->intercom_ch, command, sizeof(*command), FuriWaitForever);

    furi_check(tx_size == sizeof(*command), "Failed to send data");
}

static float status_lights_brightness_to_float(StatusLightsBrightness brightness) {
    return 0.01f * brightness.val;
}

static void status_lights_do_init(StatusLights* instance, StatusLightsApiMessage* message) {
    UNUSED(message);
    instance->intercom_ch =
        intercom_channel_open(instance->intercom, IntercomChannelIdStatusLights, NULL, NULL);
}

static void
    status_lights_do_set_brightness(StatusLights* instance, StatusLightsApiMessage* message) {
    const StatusLightsApiMessageSetBrightness* set_brightness = &message->set_brightness;

    instance->brightness = set_brightness->brightness;

    const StatusLightsCommand command = {
        .id = StatusLightsCommandIdSetBrightness,
        .as_set_brightness =
            {
                .brightness = status_lights_brightness_to_float(instance->brightness),
            },
    };

    status_lights_send_command(instance, &command);
}

static void
    status_lights_do_get_brightness(StatusLights* instance, StatusLightsApiMessage* message) {
    StatusLightsApiMessageGetBrightness* get_brightness = &message->get_brightness;

    *get_brightness->brightness = instance->brightness;
}

static void status_lights_do_run_preset(StatusLights* instance, StatusLightsApiMessage* message) {
    const StatusLightsApiMessageRunPreset* run_preset = &message->run_preset;

    const StatusLightsCommand command = {
        .id = StatusLightsCommandIdRunPreset,
        .as_run_preset =
            {
                .preset = run_preset->preset,
                .color = run_preset->color,
            },
    };

    status_lights_send_command(instance, &command);
}

static void status_lights_intercom_state_callback(const void* item, void* context) {
    furi_assert(item);
    furi_assert(context);

    StatusLights* instance = context;
    const IntercomStatus intercom_status = *(IntercomStatus*)item;

    if(intercom_status == IntercomStatusOk) {
        status_lights_init(instance);
    }
}

static void status_lights_message_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    StatusLights* instance = context;

    furi_assert(object == instance->message_queue);

    StatusLightsApiMessage message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    api_message_handlers[message.type](instance, &message);

    if(message.api_lock) {
        api_lock_unlock(message.api_lock);
    }
}

static StatusLights* status_lights_alloc() {
    StatusLights* instance = malloc(sizeof(*instance));

    instance->brightness = (StatusLightsBrightness){STATUS_LIGHTS_BRIGHTNESS_DEFAULT};

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(8, sizeof(StatusLightsApiMessage));
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        status_lights_message_callback,
        instance);

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
    [StatusLightsApiMessageTypeInit] = status_lights_do_init,
    [StatusLightsApiMessageTypeSetBrightness] = status_lights_do_set_brightness,
    [StatusLightsApiMessageTypeGetBrightness] = status_lights_do_get_brightness,
    [StatusLightsApiMessageTypeRunPreset] = status_lights_do_run_preset,
};

static_assert(COUNT_OF(api_message_handlers) == StatusLightsApiMessageTypeMax);
