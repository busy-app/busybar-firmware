#include "status_lights.h"

#include <furi/furi.h>

#include <intercom/intercom.h>

struct StatusLights {
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
    Intercom* intercom;
};

static void status_light_command_callback(FuriEventLoopObject* object, void* context) {
    StatusLights* instance = context;
    furi_check(object == instance->command_queue);

    StatusLightsCommand command;
    furi_check(
        furi_message_queue_get(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);

    size_t tx_size = intercom_tx(
        instance->intercom,
        IntercomChannelStatusLights,
        &command,
        sizeof(StatusLightsCommand),
        FuriWaitForever);

    furi_check(tx_size == sizeof(StatusLightsCommand), "Failed to send data");
}

static StatusLights* status_light_alloc() {
    StatusLights* instance = malloc(sizeof(StatusLights));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(8, sizeof(StatusLightsCommand));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        status_light_command_callback,
        instance);
    instance->intercom = furi_record_open(RECORD_INTERCOM);

    furi_record_create(RECORD_STATUS_LIGHTS, instance);

    return instance;
}

int32_t status_lights_srv(void* p) {
    UNUSED(p);

    StatusLights* instance = status_light_alloc();
    furi_event_loop_run(instance->event_loop);

    return 0;
}

void status_lights_send_command(StatusLights* instance, StatusLightsCommand command) {
    furi_check(instance);

    furi_check(
        furi_message_queue_put(instance->command_queue, &command, FuriWaitForever) ==
        FuriStatusOk);
}
