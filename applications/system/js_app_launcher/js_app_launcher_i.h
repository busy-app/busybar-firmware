#pragma once

#include "js_app_launcher.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include <js_runner/js_runner.h>

#include <js_app/js_app.h>
#include <js_app/js_app_settings_storage.h>

#define TAG "JsAppLauncher"

#define JS_APP_LAUNCHER_APP_ID "js_app_launcher"

#define JS_APP_LAUNCHER_ARG_SKIP_MENU "+"

#define RECORD_JS_APP_LAUNCHER JS_APP_LAUNCHER_APP_ID

typedef enum {
    JsAppLauncherErrorNone,
    JsAppLauncherErrorLoadFailed,
    JsAppLauncherErrorSettingsSchemaMissing,
    JsAppLauncherErrorSettingsSchemaInvalid,
    JsAppLauncherErrorSettingsStorageFailure,
    JsAppLauncherErrorSettingsLoadFailed,
    JsAppLauncherErrorSyntaxError,
    JsAppLauncherErrorProgramCrashed,
    JsAppLauncherErrorMax,
} JsAppLauncherError;

typedef struct {
    struct {
        const char* front;
        const char* back;
    } primary;
    struct {
        const char* back;
    } auxiliary;
} JsAppLauncherErrorDesc;

typedef enum {
    JsAppLauncherApiMessageTypeInvalid,
    JsAppLauncherApiMessageTypeStop,
    JsAppLauncherApiMessageTypeMax,
} JsAppLauncherApiMessageType;

typedef struct {
    JsAppLauncherStopMode mode;
} JsAppLauncherApiMessageStop;

typedef struct {
    JsAppLauncherApiMessageType type;
    union {
        JsAppLauncherApiMessageStop stop;
    };
} JsAppLauncherApiMessage;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    FuriMessageQueue* api_queue;
    SceneManager* scene_manager;
    Gui* gui;

    Widget* front_window;
    Widget* back_window;
    FlexLayout* back_container;
    NavBar* nav_bar;

    JsApp* js_app;
    JsAppSettingsStorage* settings_storage;
    JsAppLauncherError error;
    JsAppLauncherStartMode mode;
} JsAppLauncher;

typedef enum {
    JsAppLauncherCustomEventIndexMax = 0x7F,
    JsAppLauncherCustomEventScriptStarted,
    JsAppLauncherCustomEventScriptFinished,
    JsAppLauncherCustomEventSettingsChanged,
} JsAppLauncherCustomEvent;

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event);

const JsAppLauncherErrorDesc* js_app_launcher_get_error_desc(const JsAppLauncher* instance);

JsAppLauncherError js_app_launcher_translate_from_js_runner_error(JsRunnerError js_runner_error);
