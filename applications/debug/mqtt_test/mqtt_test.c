#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>

#include <mqtt_client/mqtt_client.h>

#define TAG "MqttTest"

typedef enum {
    MqttTestAppEventExit,
    MqttTestAppEventStatusUpdate,
    MqttTestAppEventPinCode,
    MqttTestAppEventLink,
    MqttTestAppEventUnlink,
} MqttTestAppEventType;

typedef struct {
    MqttTestAppEventType type;
    FuriString* str_param;
} MqttTestAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Gui* gui;

    Label* front_label;

    FlexLayout* flex;
    Label* label_status;
    Label* label_id;
    Label* label_message;

    MqttClient* mqtt;
    FuriPubSubSubscription* mqtt_event_sub;
    MqttClientStatus status;

    FuriString* session_id;
} MqttTestApp;

static void mqtt_test_update_status(MqttTestApp* instance) {
    instance->status = mqtt_client_get_status(instance->mqtt);

    if(instance->status == MqttClientStatusConnectedLinked) {
        mqtt_client_get_session_info(instance->mqtt, instance->session_id, NULL, NULL);
    } else {
        furi_string_reset(instance->session_id);
    }

    with_gui(instance->gui, {
        switch(instance->status) {
        case MqttClientStatusError:
            label_set_text(instance->label_status, "Error");
            label_set_text(instance->label_message, "Certs missing");
            break;
        case MqttClientStatusNotConnected:
            label_set_text(instance->label_status, "Not connected");
            label_set_text(instance->label_message, "");
            break;
        case MqttClientStatusConnectedNotLinked:
            label_set_text(instance->label_status, "Connected, not linked");
            label_set_text(instance->label_message, "Press start to request link pin");
            break;
        case MqttClientStatusConnectedLinked:
            label_set_text(instance->label_status, "Connected, linked");
            label_set_text(instance->label_message, "Hold start to unlink");
            break;
        default:
            furi_crash();
        }

        if(!furi_string_empty(instance->session_id)) {
            label_set_text_fmt(
                instance->label_id, "ID: %s", furi_string_get_cstr(instance->session_id));
        } else {
            label_set_text(instance->label_id, "");
        }
    });
}

static bool mqtt_test_app_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    MqttTestApp* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        consumed = true;
        const MqttTestAppEvent app_event = {.type = MqttTestAppEventExit};
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    } else if(event->key == InputKeyStart) {
        if(event->type == InputTypeShort) {
            consumed = true;
            const MqttTestAppEvent app_event = {.type = MqttTestAppEventLink};
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        } else if(event->type == InputTypeLong) {
            consumed = true;
            const MqttTestAppEvent app_event = {.type = MqttTestAppEventUnlink};
            furi_check(
                furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
                FuriStatusOk);
        }
    }

    return consumed;
}

static void mqtt_test_app_event_queue_callback(FuriEventLoopObject* object, void* context) {
    MqttTestApp* instance = context;
    furi_check(object == instance->event_queue);

    MqttTestAppEvent event;
    furi_check(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk);

    if(event.type == MqttTestAppEventExit) {
        furi_event_loop_stop(instance->event_loop);
    } else if(event.type == MqttTestAppEventStatusUpdate) {
        mqtt_test_update_status(instance);
    } else if(event.type == MqttTestAppEventPinCode) {
        label_set_text_fmt(
            instance->label_id, "Link PIN: %s", furi_string_get_cstr(event.str_param));
        furi_string_free(event.str_param);
    } else if(
        event.type == MqttTestAppEventLink &&
        (instance->status == MqttClientStatusConnectedNotLinked)) {
        mqtt_client_request_link_pin(instance->mqtt);
    } else if(event.type == MqttTestAppEventUnlink) {
        mqtt_client_unlink(instance->mqtt);
    }
}

static void mqtt_test_events_callback(const void* message, void* context) {
    MqttTestApp* instance = context;
    furi_assert(instance);

    const MqttEvent* mqtt_event = (const MqttEvent*)message;
    furi_assert(mqtt_event);

    if(mqtt_event->type == MqttEventTypeStatusChanged) {
        const MqttTestAppEvent app_event = {
            .type = MqttTestAppEventStatusUpdate,
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    } else if(mqtt_event->type == MqttEventTypeLinkPinReceived) {
        const MqttTestAppEvent app_event = {
            .type = MqttTestAppEventPinCode,
            .str_param = furi_string_alloc_set_str(mqtt_event->link_pin_received.pin),
        };
        furi_check(
            furi_message_queue_put(instance->event_queue, &app_event, FuriWaitForever) ==
            FuriStatusOk);
    }
}

static MqttTestApp* mqtt_test_app_alloc(void) {
    MqttTestApp* instance = malloc(sizeof(MqttTestApp));
    instance->session_id = furi_string_alloc();

    instance->mqtt = furi_record_open(RECORD_MQTT);
    instance->mqtt_event_sub = furi_pubsub_subscribe(
        mqtt_client_get_pubsub(instance->mqtt), mqtt_test_events_callback, instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->event_queue = furi_message_queue_alloc(16, sizeof(MqttTestAppEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        mqtt_test_app_event_queue_callback,
        instance);

    instance->gui = furi_record_open(RECORD_GUI);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, mqtt_test_app_input_callback, instance);

        // Front display
        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->front_label = label_alloc(root);
        label_set_text(instance->front_label, "Look at back display");
        widget_set_align(label_get_base(instance->front_label), AlignCenter);

        // Back display
        root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);
        instance->flex = flex_layout_alloc(root, FlexLayoutTypeColumn);
        Widget* flex_base = flex_layout_get_base(instance->flex);

        instance->label_status = label_alloc(flex_base);
        instance->label_id = label_alloc(flex_base);
        instance->label_message = label_alloc(flex_base);
    });
    mqtt_test_update_status(instance);

    return instance;
}

static void mqtt_test_app_free(MqttTestApp* instance) {
    furi_check(instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, mqtt_test_app_input_callback);

        label_free(instance->label_id);
        label_free(instance->label_message);
        label_free(instance->label_status);

        label_free(instance->front_label);

        flex_layout_free(instance->flex);
    });

    furi_record_close(RECORD_GUI);

    furi_pubsub_unsubscribe(mqtt_client_get_pubsub(instance->mqtt), instance->mqtt_event_sub);
    furi_record_close(RECORD_MQTT);

    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->event_queue);
    furi_event_loop_free(instance->event_loop);

    furi_string_free(instance->session_id);

    free(instance);
}

int32_t mqtt_test_app(void* args) {
    UNUSED(args);

    MqttTestApp* instance = mqtt_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    mqtt_test_app_free(instance);

    return 0;
}
