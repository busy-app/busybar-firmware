#include "js_app_launcher_i.h"

static const JsAppLauncherErrorDesc js_app_launcher_error_descs[JsAppLauncherErrorMax] = {
    [JsAppLauncherErrorNone] =
        {
            {0}, /* Empty, no error message to display */
            {0}, /* Empty, no error message to display */
        },
    [JsAppLauncherErrorLoadFailed] =
        {
            .primary =
                {
                    .front = "App loading failed,\nreinstall it.",
                    .back = "App loading failed",
                },
            .auxiliary =
                {
                    .back = "Try to restart or reinstall it",
                },
        },
    [JsAppLauncherErrorSyntaxError] =
        {
            .primary =
                {
                    .front = "Syntax error,\ncheck script.",
                    .back = "Syntax error",
                },
            .auxiliary =
                {
                    .back = "Check the app script file",
                },
        },
    [JsAppLauncherErrorProgramCrashed] =
        {
            .primary =
                {
                    .front = "App crashed,\nsee device logs.",
                    .back = "App crashed",
                },
            .auxiliary =
                {
                    .back = "See device logs",
                },
        },
};

const JsAppLauncherErrorDesc* js_app_launcher_get_error_desc(const JsAppLauncher* instance) {
    furi_assert(instance->error < JsAppLauncherErrorMax);
    furi_assert(instance->error != JsAppLauncherErrorNone);
    return &js_app_launcher_error_descs[instance->error];
}

JsAppLauncherError js_app_launcher_translate_from_js_runner_error(JsRunnerError js_runner_error) {
    furi_assert(js_runner_error < JsRunnerErrorMax);
    JsAppLauncherError translated_error;

    if(js_runner_error == JsRunnerErrorParseException) {
        translated_error = JsAppLauncherErrorSyntaxError;
    } else {
        translated_error = JsAppLauncherErrorLoadFailed;
    }

    return translated_error;
}
