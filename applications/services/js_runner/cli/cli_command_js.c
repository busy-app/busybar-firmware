#include "cli_command_js.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <js_runner/js_runner.h>
#include <argparse.h>

typedef struct JsCliParams {
    FuriString* script_path;
    FuriString* app_id;
    bool abort_scripts;
} JsCliParams;

#define CLI_APP_ID        "app.busy.cli"
#define CLI_APP_HEAP_SIZE 64 * 1024

static void js_console_cb(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context) {

    PipeSide* pipe = context;

    static const char* const preamble[] = {
        [JsRunnerConsoleSeverityLog] = "",
        [JsRunnerConsoleSeverityError] = "\x1b[1;31m",
        [JsRunnerConsoleSeverityInfo] = "\x1b[1;33m",
    };
    static const char* const postamble[] = {
        [JsRunnerConsoleSeverityLog] = "",
        [JsRunnerConsoleSeverityError] = "\x1b[0m",
        [JsRunnerConsoleSeverityInfo] = "\x1b[0m",
    };
    pipe_send(pipe, preamble[severity], strlen(preamble[severity]));
    pipe_send(pipe, buf, size);
    pipe_send(pipe, postamble[severity], strlen(postamble[severity]));
    switch(separator) {
    case JsRunnerConsoleSeparatorNone:
        break;
    case JsRunnerConsoleSeparatorSpace:
        pipe_send(pipe, " ", 1);
        break;
    case JsRunnerConsoleSeparatorNewline:
        pipe_send(pipe, "\r\n", 2);
        break;
    }
}

static void run_script(const FuriString* arg, const FuriString* app_id, PipeSide* pipe) {
    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    const char* id = app_id ? furi_string_get_cstr(app_id) : CLI_APP_ID;
    JsRunnerContextInitResult context_init_result =
        js_runner_context_alloc(runner, id, CLI_APP_HEAP_SIZE, js_console_cb, pipe);
    if(context_init_result.error != JsRunnerErrorNone) {
        printf(
            "Error allocating JS context: %s",
            js_runner_get_error_message(context_init_result.error));
    } else {
        JsRunnerContextHandle* handle = context_init_result.handle;
        JsRunnerRunResult run_result = js_runner_run(handle, furi_string_get_cstr(arg));
        if(run_result.error != JsRunnerErrorNone) {
            printf("Error running script: %s", js_runner_get_error_message(run_result.error));
        } else {
            JsRunnerError join_result = js_runner_join(run_result.handle, FuriWaitForever);
            furi_check(join_result == JsRunnerErrorNone);
        }
        js_runner_context_free(handle);
    }
    furi_record_close(RECORD_JS_RUNNER);
}

static void abort_all(void) {
    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    js_runner_abort_all(runner);
    furi_record_close(RECORD_JS_RUNNER);
}

static void argparse_callback(char opt, const char* optarg, void* context) {
    JsCliParams* params = context;
    switch(opt) {
    case 'i': {
        params->app_id = furi_string_alloc_set(optarg);
        break;
    }
    case 'k': {
        params->abort_scripts = true;
        break;
    }
    case 0: {
        params->script_path = furi_string_alloc_set(optarg);
        break;
    }
    default:
        furi_check(false);
        break;
    }
}

static bool validate_params(const JsCliParams* params) {
    bool has_abort = params->abort_scripts;
    bool has_run = params->script_path || params->app_id;
    return has_abort != has_run;
}

void cli_command_js(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    JsCliParams params = {0};
    bool parse_ok = parse_args(args, "i:k", argparse_callback, &params);
    bool validate_ok = validate_params(&params);
    if(!parse_ok || !validate_ok) {
        printf("Usage: js [-i app_id] <filename>\r\n");
        printf("       js -k\r\n");
    } else {
        if(params.abort_scripts) {
            abort_all();
        } else if(params.script_path) {
            run_script(params.script_path, params.app_id, pipe);
        }
    }
    if(params.app_id) {
        furi_string_free(params.app_id);
    }
    if(params.script_path) {
        furi_string_free(params.script_path);
    }
}
