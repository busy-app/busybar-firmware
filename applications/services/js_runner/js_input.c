#include "js_input.h"
#include "js_util.h"

#include <input/input.h>

#define TAG "JsInput"

typedef enum JsInputControl {
    JsInputControlEncoder,
    JsInputControlStart,
    JsInputControlOk,
    JsInputControlBack,
    JsInputControlMax,
} JsInputControl;

static const char* control_names[JsInputControlMax] = {
    [JsInputControlEncoder] = "encoder",
    [JsInputControlStart] = "start",
    [JsInputControlOk] = "ok",
    [JsInputControlBack] = "back",
};

static const char* button_actions[InputTypeMAX] = {
    [InputTypePress] = "press",
    [InputTypeRelease] = "release",
};

static bool js_input_control_from_event(const InputEvent* event, JsInputControl* control) {
    switch(event->key) {
    case InputKeyUp:
    case InputKeyDown:
        if(event->type != InputTypePress) return false;
        *control = JsInputControlEncoder;
        return true;
    case InputKeyBack:
        if((event->type != InputTypePress) && (event->type != InputTypeRelease)) return false;
        *control = JsInputControlBack;
        return true;
    case InputKeyStart:
        if((event->type != InputTypePress) && (event->type != InputTypeRelease)) return false;
        *control = JsInputControlStart;
        return true;
    case InputKeyOk:
        if((event->type != InputTypePress) && (event->type != InputTypeRelease)) return false;
        *control = JsInputControlOk;
        return true;
    default:
        return false;
    }
}

static inline bool js_input_listener_attached(JsRunnerAppInput* input) {
    return input->listen_handler != 0;
}

static void input_event_handler(const void* message, void* context) {
    const InputEvent* event = message;
    JsRunnerApp* app = context;

    JsInputControl control = JsInputControlMax;
    if(!js_input_control_from_event(event, &control)) return;

    furi_check(
        furi_message_queue_put(app->input.input_queue, event, FuriWaitForever) == FuriStatusOk);
}

static jerry_value_t js_input_create_event(const InputEvent* event, JsInputControl control) {
    jerry_value_t js_event = jerry_object();
    furi_check(event->type < InputTypeMAX);

    js_set_property(js_event, "key", jerry_string_sz(control_names[control]));

    if(control == JsInputControlEncoder) {
        const bool clockwise = event->key == InputKeyUp;
        js_set_property(
            js_event, "action", jerry_string_sz(clockwise ? "clockwise" : "counterclockwise"));
        js_set_property(js_event, "delta", jerry_number(clockwise ? 1 : -1));
    } else {
        js_set_property(js_event, "action", jerry_string_sz(button_actions[event->type]));
    }

    return js_event;
}

static void js_input_queue_handler(FuriEventLoopObject* object, void* context) {
    JsRunnerApp* app = context;
    furi_check(object == app->input.input_queue);

    InputEvent event;
    furi_check(furi_message_queue_get(app->input.input_queue, &event, 0) == FuriStatusOk);

    JsInputControl control = JsInputControlMax;
    if(!js_input_control_from_event(&event, &control)) {
        return;
    }

    jerry_value_t js_event = js_input_create_event(&event, control);

    jerry_value_t js_result =
        jerry_call(app->input.listen_handler, jerry_undefined(), &js_event, 1);

    if(jerry_value_is_exception(js_result)) {
        js_log_exception(TAG, "Exception", js_result);
    }
    js_run_jobs();
    jerry_value_free(js_result);
    jerry_value_free(js_event);
}

static void js_input_listen(JsRunnerApp* app) {
    if(app->input.pubsub_subscription) {
        return;
    }

    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    app->input.pubsub_subscription = furi_pubsub_subscribe(input_events, input_event_handler, app);
    furi_record_close(RECORD_INPUT_EVENTS);

    furi_event_loop_subscribe_message_queue(
        app->event_loop, app->input.input_queue, FuriEventLoopEventIn, js_input_queue_handler, app);
}

static void js_input_unbind(JsRunnerApp* app) {
    if(!js_input_listener_attached(&app->input) ||
       !jerry_value_is_function(app->input.listen_handler)) {
        return;
    }

    jerry_value_free(app->input.listen_handler);
    app->input.listen_handler = 0;

    furi_event_loop_unsubscribe(app->event_loop, app->input.input_queue);
    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_pubsub_unsubscribe(input_events, app->input.pubsub_subscription);
    furi_record_close(RECORD_INPUT_EVENTS);
    app->input.pubsub_subscription = NULL;
}

static jerry_value_t unbind(
    const jerry_call_info_t* call_info_p,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info_p);
    UNUSED(args);
    UNUSED(args_count);

    WITH_JS_RUNNER_APP(app, {
        js_input_unbind(app);
        js_runner_app_stop_if_done(app);
    });

    return jerry_undefined();
}

static jerry_value_t listen(
    const jerry_call_info_t* call_info_p,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info_p);
    JS_CHECK_ARGS_COUNT(2);

    JS_CHECK_ARG_IS_STRING(JS_ARG(0));
    JS_CHECK_ARG_IS_FUNCTION(JS_ARG(1));

    FuriString* type = js_string_to_furi_string(JS_ARG(0));
    if(!furi_string_equal_str(type, "input")) {
        furi_string_free(type);
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Unknown event type");
    }
    furi_string_free(type);

    WITH_JS_RUNNER_APP(app, {
        if(js_input_listener_attached(&app->input) &&
           jerry_value_is_function(app->input.listen_handler)) {
            return jerry_throw_sz(JERRY_ERROR_TYPE, "Handler override is forbidden");
        }

        app->input.listen_handler = jerry_value_copy(JS_ARG(1));
        js_input_listen(app);
    });

    return jerry_function_external(unbind);
}

void js_setup_input_methods(void) {
    jerry_value_t global_obj = jerry_current_realm();

    js_set_method(global_obj, "listen", listen);

    jerry_value_free(global_obj);
}

void js_runner_app_input_init(JsRunnerAppInput* instance) {
    furi_assert(instance);
    memset(instance, 0, sizeof(JsRunnerAppInput));
    instance->input_queue = furi_message_queue_alloc(10, sizeof(InputEvent));
}

void js_runner_app_input_abort(JsRunnerAppInput* instance) {
    furi_assert(instance);
    if(instance->pubsub_subscription == NULL) return;

    WITH_JS_RUNNER_APP(app, { js_input_unbind(app); });
}

void js_runner_app_input_deinit(JsRunnerAppInput* instance) {
    furi_assert(instance);

    furi_message_queue_free(instance->input_queue);
}
