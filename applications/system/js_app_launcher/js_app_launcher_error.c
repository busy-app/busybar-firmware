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
                    .front = "App loading failed,\nreinstall it.",
                    .back = "App loading failed",
                },
            .auxiliary =
                {
                    .back = "Try to restart or reinstall it",
                },
        },
    [JsAppLauncherShowStatusApiVersionMismatch] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "App incompatible,\nupdate firmware.",
                    .back = "App not compatible",
                },
            .auxiliary =
                {
                    .back = "Update the firmware",
                },
        },
    [JsAppLauncherShowStatusSettingsSchemaMissing] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralInfo,
            .primary =
                {
                    .front = "This app has\nno settings.",
                    .back = "No settings",
                },
            .auxiliary =
                {
                    .back = "Add settings file to the app",
                },
        },
    [JsAppLauncherShowStatusSettingsSchemaInvalid] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Settings file\nis invalid.",
                    .back = "Invalid settings",
                },
            .auxiliary =
                {
                    .back = "Fix the app settings file",
                },
        },
    [JsAppLauncherShowStatusSettingsStorageFailure] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Settings storage\nerror.",
                    .back = "Settings storage error",
                },
            .auxiliary =
                {
                    .back = "Check the storage device",
                },
        },
    [JsAppLauncherShowStatusSettingsLoadFailed] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
            .primary =
                {
                    .front = "Settings loading\nfailed.",
                    .back = "Settings loading failed",
                },
            .auxiliary =
                {
                    .back = "Check the app settings file",
                },
        },
    [JsAppLauncherShowStatusSyntaxError] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
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
    [JsAppLauncherShowStatusProgramCrashed] =
        {
            .type = JsAppLauncherShowStatusTypeGeneralError,
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
