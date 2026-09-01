#include "js_input.h"

#include <input/input.h>

#define TAG "JsInput"

static const char* control_names[JsInputControlMax] = {
    [JsInputControlDial] = "dial",
    [JsInputControlStartPause] = "startPause",
    [JsInputControlOk] = "ok",
};

static const char* button_actions[InputTypeMAX] = {
    [InputTypePress] = "press",
    [InputTypeRelease] = "release",
};

static bool js_input_control_from_name(const char* name, JsInputControl* control) {
    for(size_t i = 0; i < JsInputControlMax; i++) {
        if(strcmp(name, control_names[i]) == 0) {
            *control = i;
            return true;
        }
    }
    return false;
}

static bool js_input_control_from_event(const InputEvent* event, JsInputControl* control) {
    switch(event->key) {
    case InputKeyUp:
    case InputKeyDown:
        if(event->type != InputTypePress) return false;
        *control = JsInputControlDial;
        return true;
    case InputKeyStart:
        if((event->type != InputTypePress) && (event->type != InputTypeRelease)) return false;
        *control = JsInputControlStartPause;
        return true;
    case InputKeyOk:
        if((event->type != InputTypePress) && (event->type != InputTypeRelease)) return false;
        *control = JsInputControlOk;
        return true;
    default:
        return false;
    }
}

static jerry_value_t js_input_create_event(const InputEvent* event, JsInputControl control) {
    jerry_value_t js_event = jerry_object();

    if(control == JsInputControlDial) {
        const bool clockwise = event->key == InputKeyUp;
        js_set_property(
            js_event, "direction", jerry_string_sz(clockwise ? "clockwise" : "counterclockwise"));
        js_set_property(js_event, "delta", jerry_number(clockwise ? 1 : -1));
    } else {
        furi_check(event->type < InputTypeMAX);
        js_set_property(js_event, "action", jerry_string_sz(button_actions[event->type]));
    }

    return js_event;
}

static void js_input_process_event(JsRunnerApp* app, const InputEvent* event) {
    JsInputControl control;
    if(!js_input_control_from_event(event, &control) || !app->input.handler_registered[control]) {
        return;
    }

    jerry_value_t js_event = js_input_create_event(event, control);
    jerry_value_t result =
        jerry_call(app->input.handlers[control], jerry_undefined(), &js_event, 1);
    if(jerry_value_is_exception(result)) {
        js_report_uncaught_exception(app, result);
    }
    jerry_value_free(result);
    jerry_value_free(js_event);
    js_run_jobs();
}

static void js_input_event_queue_callback(FuriEventLoopObject* object, void* context) {
    JsRunnerApp* app = context;
    furi_assert(object == app->input.event_queue);

    InputEvent event;
    while(furi_message_queue_get(app->input.event_queue, &event, 0) == FuriStatusOk) {
        js_input_process_event(app, &event);
    }
}

static void js_input_callback(const void* message, void* context) {
    JsRunnerApp* app = context;
    const InputEvent* event = message;
    JsInputControl control;
    if(!js_input_control_from_event(event, &control)) return;

    if(furi_message_queue_put(app->input.event_queue, event, 0) != FuriStatusOk) {
        // A slow handler can back the queue up. Dropping a press is survivable;
        // dropping its release strands any script tracking held state, so make
        // room by discarding the oldest event rather than the new one.
        InputEvent dropped;
        if(furi_message_queue_get(app->input.event_queue, &dropped, 0) == FuriStatusOk) {
            FURI_LOG_W(TAG, "Input queue full, dropped oldest event");
            furi_message_queue_put(app->input.event_queue, event, 0);
        }
    }
}

static void js_input_subscribe(JsRunnerApp* app) {
    if(app->input.subscription) return;

    app->input.events = furi_record_open(RECORD_INPUT_EVENTS);
    app->input.subscription = furi_pubsub_subscribe(app->input.events, js_input_callback, app);
}

static jerry_value_t js_input_on(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);
    JS_CHECK_ARGS_COUNT(2);

    char* name = js_string_to_c_string(args[0]);
    if(!name) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Control name must be a string");
    }

    JsInputControl control;
    const bool valid_name = js_input_control_from_name(name, &control);
    free(name);
    if(!valid_name) {
        return jerry_throw_sz(JERRY_ERROR_RANGE, "Unknown control name");
    }
    if(!jerry_value_is_function(args[1])) {
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Control handler must be a function");
    }

    WITH_JS_RUNNER_APP(app, {
        if(app->input.handler_registered[control]) {
            jerry_value_free(app->input.handlers[control]);
        }
        app->input.handlers[control] = jerry_value_copy(args[1]);
        app->input.handler_registered[control] = true;
        js_input_subscribe(app);
    });

    return jerry_undefined();
}

void js_runner_app_input_init(JsRunnerApp* app) {
    app->input.event_queue = furi_message_queue_alloc(MAX_INPUT_MESSAGES, sizeof(InputEvent));
    app->input.events = NULL;
    app->input.subscription = NULL;
    for(size_t i = 0; i < JsInputControlMax; i++) {
        app->input.handler_registered[i] = false;
    }

    furi_event_loop_subscribe_message_queue(
        app->event_loop,
        app->input.event_queue,
        FuriEventLoopEventIn,
        js_input_event_queue_callback,
        app);
}

void js_runner_app_input_deinit(JsRunnerApp* app) {
    for(size_t i = 0; i < JsInputControlMax; i++) {
        furi_check(!app->input.handler_registered[i]);
    }

    if(app->input.subscription) {
        furi_pubsub_unsubscribe(app->input.events, app->input.subscription);
        furi_record_close(RECORD_INPUT_EVENTS);
    }
    furi_event_loop_unsubscribe(app->event_loop, app->input.event_queue);
    furi_message_queue_free(app->input.event_queue);
}

void js_setup_input(void) {
    jerry_value_t global_obj = jerry_current_realm();
    jerry_value_t input = jerry_object();
    js_set_method(input, "on", js_input_on);
    js_set_property(global_obj, "Input", input);
    jerry_value_free(global_obj);
}
