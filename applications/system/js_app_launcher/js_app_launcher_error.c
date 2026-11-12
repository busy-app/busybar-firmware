#include "js_app_launcher_i.h"

static const JsAppLauncherShowStatusDesc js_app_launcher_show_status_descs[] = {
    [JsAppLauncherShowStatusNone] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary = {0}, /* Empty, no error message to display */
            .auxiliary = {0}, /* Empty, no error message to display */
        },
    [JsAppLauncherShowStatusLoadFailed] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "App failed to load,\nrestart app",
                    .back = "App failed to load",
                },
            .auxiliary =
                {
                    .back = "Restart app",
                },
        },
    [JsAppLauncherShowStatusApiVersionMismatch] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "App not supported\nUpdate firmware",
                    .back = "App not supported",
                },
            .auxiliary =
                {
                    .back = "Update firmware to use it",
                },
        },
    [JsAppLauncherShowStatusSettingsSchemaMissing] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralInfo,
            .primary =
                {
                    .front = "No settings\nto adjust",
                    .back = "No settings",
                },
            .auxiliary =
                {
                    .back = "Nothing to set up in this app",
                },
        },
    [JsAppLauncherShowStatusSettingsSchemaInvalid] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Invalid settings\nfile",
                    .back = "Invalid settings file",
                },
            .auxiliary =
                {
                    .back = "Reinstall the app or fix\nthe settings.json file",
                },
        },
    [JsAppLauncherShowStatusSettingsStorageFailure] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Storage error\nRestart device",
                    .back = "Storage error",
                },
            .auxiliary =
                {
                    .back = "Restart BUSY Bar and retry",
                },
        },
    [JsAppLauncherShowStatusSettingsLoadFailed] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Load error\nRestart device",
                    .back = "Cannot load app settings",
                },
            .auxiliary =
                {
                    .back = "Restart BUSY Bar and retry",
                },
        },
    [JsAppLauncherShowStatusSyntaxError] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Syntax error",
                    .back = "Syntax error",
                },
            .auxiliary =
                {
                    .back = "Check the app script file",
                },
        },
    [JsAppLauncherShowStatusProgramCrashed] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "App crashed,\nrestart app",
                    .back = "App crashed",
                },
            .auxiliary =
                {
                    .back = "Restart app",
                },
        },
};

static_assert(COUNT_OF(js_app_launcher_show_status_descs) == JsAppLauncherShowStatusMax);

const JsAppLauncherShowStatusDesc*
    js_app_launcher_get_show_status_desc(const JsAppLauncher* instance) {
    furi_assert(instance->show_status < JsAppLauncherShowStatusMax);
    furi_assert(instance->show_status != JsAppLauncherShowStatusNone);
    return &js_app_launcher_show_status_descs[instance->show_status];
}

JsAppLauncherShowStatus
    js_app_launcher_translate_from_js_runner_error(JsRunnerError js_runner_error) {
    furi_assert(js_runner_error < JsRunnerErrorMax);
    JsAppLauncherShowStatus translated_status;

    if(js_runner_error == JsRunnerErrorNone) {
        translated_status = JsAppLauncherShowStatusNone;
    } else if(js_runner_error == JsRunnerErrorParseException) {
        translated_status = JsAppLauncherShowStatusSyntaxError;
    } else if(js_runner_error == JsRunnerErrorOutOfMemory) {
        translated_status = JsAppLauncherShowStatusProgramCrashed;
    } else {
        translated_status = JsAppLauncherShowStatusLoadFailed;
    }

    return translated_status;
}
