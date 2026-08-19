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

#define CLI_APP_ID "app.busy.cli"

static void js_console_cb(
    JsRunnerConsoleSeverity severity,
    const char* buf,
    size_t size,
    JsRunnerConsoleSeparator separator,
    void* context) {
    UNUSED(context);

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
    printf("%s", preamble[severity]);
    printf("%.*s", size, buf);
    printf("%s", postamble[severity]);
    switch(separator) {
    case JsRunnerConsoleSeparatorNone:
        break;
    case JsRunnerConsoleSeparatorSpace:
        printf(" ");
        break;
    case JsRunnerConsoleSeparatorNewline:
        printf("\r\n");
        break;
    }
}

static void run_script(const FuriString* arg, const FuriString* app_id) {
    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    const char* id = app_id ? furi_string_get_cstr(app_id) : CLI_APP_ID;
    JsRunnerError error =
        js_runner_run(runner, id, furi_string_get_cstr(arg), 64 * 1024, js_console_cb, NULL);
    if(error != JsRunnerErrorNone) {
        printf("Error running script: %s", js_runner_get_error_message(error));
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
            run_script(params.script_path, params.app_id);
        }
    }
    if(params.app_id) {
        furi_string_free(params.app_id);
    }
    if(params.script_path) {
        furi_string_free(params.script_path);
    }
}
