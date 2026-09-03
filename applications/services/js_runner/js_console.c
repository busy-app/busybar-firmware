#include "js_console.h"

#define TAG "JsConsole"

typedef struct {
    JsRunnerConsoleOutCallback write;
    void* context;
    JsRunnerConsoleSeverity severity;
} ConsoleContext;

static void console_context_free(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);
    free(native_p);
}

static const jerry_object_native_info_t console_native_info = {
    .free_cb = console_context_free,
};

static jerry_value_t console_log(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    ConsoleContext* ctx = jerry_object_get_native_ptr(call_info->function, &console_native_info);
    JsRunnerConsoleSeverity severity = ctx->severity;

    furi_check(ctx);

    if(ctx->write == NULL) {
        return jerry_undefined();
    }

    for(jerry_length_t i = 0; i < args_count; i++) {
        jerry_value_t str = jerry_value_to_string(args[i]);

        jerry_size_t size = jerry_string_size(str, JERRY_ENCODING_UTF8);

        JsRunnerConsoleSeparator separator =
            i + 1 == args_count ? JsRunnerConsoleSeparatorNewline : JsRunnerConsoleSeparatorSpace;

        if(size > 0) {
            char* buf = malloc(size);

            jerry_string_to_buffer(str, JERRY_ENCODING_UTF8, (jerry_char_t*)buf, size);
            ctx->write(severity, buf, size, separator, ctx->context);

            free(buf);

        } else {
            ctx->write(severity, "", 0, separator, ctx->context);
        }

        jerry_value_free(str);
    }

    return jerry_undefined();
}

static void add_logging_method(
    jerry_value_t console_obj,
    const char* name,
    JsRunnerConsoleSeverity severity,
    JsRunnerConsoleOutCallback console_callback,
    void* console_write_context) {
    ConsoleContext* fn_context = malloc(sizeof(ConsoleContext));
    fn_context->write = console_callback;
    fn_context->context = console_write_context;
    fn_context->severity = severity;

    jerry_value_t fn = jerry_function_external(console_log);
    jerry_object_set_native_ptr(fn, &console_native_info, fn_context);

    js_check_and_free(jerry_object_set_sz(console_obj, name, fn));
    jerry_value_free(fn);
}

void js_setup_console(JsRunnerAppConsole* console) {
    JsRunnerConsoleOutCallback console_callback = console->callback;
    void* console_write_context = console->callback_context;
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t console_obj = jerry_object();
    js_check_and_free(jerry_object_set_sz(global_obj, "console", console_obj));

    add_logging_method(
        console_obj, "log", JsRunnerConsoleSeverityLog, console_callback, console_write_context);
    add_logging_method(
        console_obj, "info", JsRunnerConsoleSeverityInfo, console_callback, console_write_context);
    add_logging_method(
        console_obj,
        "error",
        JsRunnerConsoleSeverityError,
        console_callback,
        console_write_context);

    jerry_value_free(console_obj);
    jerry_value_free(global_obj);
}
