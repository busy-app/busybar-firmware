#include "cli_command_js.h"
#include <furi/furi.h>

#include <cli/args.h>
#include <js_runner/js_runner.h>

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

static void run_script(const FuriString* arg) {
    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    JsRunnerError error =
        js_runner_run(runner, furi_string_get_cstr(arg), 64 * 1024, js_console_cb, NULL);
    if(error != JsRunnerErrorNone) {
        printf("Error running script: %d", error);
    }
    furi_record_close(RECORD_JS_RUNNER);
}

static void abort_all(void) {
    JsRunner* runner = furi_record_open(RECORD_JS_RUNNER);
    js_runner_abort_all(runner);
    furi_record_close(RECORD_JS_RUNNER);
}

void cli_command_js(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(context);

    if(furi_string_size(args) > 0) {
        if(furi_string_cmp(args, "-kill") == 0) {
            abort_all();
        } else {
            run_script(args);
        }
    } else {
        printf("Usage: js <filename>\r\n");
        printf("Usage: js -kill\r\n");
    }
}
