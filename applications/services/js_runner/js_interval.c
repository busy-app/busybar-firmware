#include "js_interval.h"

#define TAG "JsInterval"

static void interval_callback(void* context) {
    uint32_t timer_id = (uint32_t)context;

    WITH_JS_RUNNER_APP(app, {
        const IntervalContext* interval_context =
            IntervalDict_cget(app->interval.intervals, timer_id);
        if(interval_context) {
            jerry_value_t result =
                jerry_call(interval_context->callback, jerry_undefined(), NULL, 0);
            if(jerry_value_is_exception(result)) {
                FuriString* exception_string = js_get_exception_string(result);

                if(exception_string) {
                    if(app->console.callback) {
                        app->console.callback(
                            JsRunnerConsoleSeverityError,
                            "Uncaught:",
                            9,
                            JsRunnerConsoleSeparatorSpace,
                            app->console.callback_context);
                        app->console.callback(
                            JsRunnerConsoleSeverityError,
                            furi_string_get_cstr(exception_string),
                            furi_string_size(exception_string),
                            JsRunnerConsoleSeparatorNewline,
                            app->console.callback_context);
                    }
                    furi_string_free(exception_string);
                } else {
                    if(app->console.callback) {
                        app->console.callback(
                            JsRunnerConsoleSeverityError,
                            "Uncaught exception",
                            18,
                            JsRunnerConsoleSeparatorNewline,
                            app->console.callback_context);
                    }
                }
            }
            jerry_value_free(result);
        } else {
            FURI_LOG_E(TAG, "Dead interval timer with id = %lu", timer_id);
        }

        js_run_jobs();

        interval_context = IntervalDict_cget(app->interval.intervals, timer_id);
        if(interval_context && interval_context->once) {
            jerry_value_free(interval_context->callback);
            furi_event_loop_timer_free(interval_context->timer);
            IntervalDict_erase(app->interval.intervals, timer_id);
            js_runner_app_stop_if_done(app);
        }
    });
}

static jerry_value_t set_interval_or_timeout(
    const jerry_value_t args[],
    const jerry_length_t args_count,
    bool once) {
    if(args_count != 2) {
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Wrong argument count");
    }

    jerry_value_t callback_val = args[0];
    float timeout_ms = jerry_value_as_number(args[1]);
    if(timeout_ms < MIN_INTERVAL_DELAY_MS) {
        timeout_ms = MIN_INTERVAL_DELAY_MS;
    }

    uint32_t timer_id = 0;
    WITH_JS_RUNNER_APP(app, {
        timer_id = app->interval.last_id;
        app->interval.last_id += 1;
    });

    IntervalContext interval_context = {
        .callback = jerry_value_copy(callback_val),
        .once = once,
    };
    WITH_JS_RUNNER_APP(app, {
        interval_context.timer = furi_event_loop_timer_alloc(
            app->event_loop,
            interval_callback,
            once ? FuriEventLoopTimerTypeOnce : FuriEventLoopTimerTypePeriodic,
            (void*)timer_id);
        IntervalDict_set_at(app->interval.intervals, timer_id, interval_context);
        furi_event_loop_timer_start(
            interval_context.timer, furi_ms_to_ticks((uint32_t)timeout_ms));
    });

    return jerry_number(timer_id);
}

static jerry_value_t set_interval(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);

    return set_interval_or_timeout(args, args_count, false);
}

static jerry_value_t set_timeout(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);
    return set_interval_or_timeout(args, args_count, true);
}

static jerry_value_t clear_interval(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(call_info);

    if(args_count != 1) {
        return jerry_throw_sz(JERRY_ERROR_COMMON, "Wrong argument count");
    }
    uint32_t timer_id = (uint32_t)jerry_value_as_number(args[0]);

    WITH_JS_RUNNER_APP(app, {
        js_interval_abort(app, timer_id);
        js_runner_app_stop_if_done(app);
    });
    return jerry_undefined();
}

void js_setup_interval_methods(void) {
    jerry_value_t global_obj = jerry_current_realm();

    js_set_method(global_obj, "setInterval", set_interval);
    js_set_method(global_obj, "setTimeout", set_timeout);
    js_set_method(global_obj, "clearInterval", clear_interval);
    js_set_method(global_obj, "clearTimeout", clear_interval);

    jerry_value_free(global_obj);
}

void js_interval_abort(JsRunnerApp* app, uint32_t id) {
    const IntervalContext* interval_context = IntervalDict_cget(app->interval.intervals, id);
    if(interval_context) {
        jerry_value_free(interval_context->callback);
        furi_event_loop_timer_free(interval_context->timer);
        IntervalDict_erase(app->interval.intervals, id);
    } else {
        FURI_LOG_E(TAG, "Interval with ID %lu is not found", id);
    }
}
