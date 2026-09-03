#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include <js_runner/js_runner.h>

#include <js_app/js_app.h>

#define TAG "JsAppLauncher"

typedef enum {
    JsAppLauncherErrorNone,
    JsAppLauncherErrorLoadFailed,
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

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    Gui* gui;

    Widget* front_window;
    Widget* back_window;
    FlexLayout* back_container;
    NavBar* nav_bar;

    JsApp* js_app;
    JsAppLauncherError error;
} JsAppLauncher;

typedef enum {
    JsAppLauncherCustomEventIndexMax = 0x7F,
    JsAppLauncherCustomEventScriptFinished,
} JsAppLauncherCustomEvent;

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event);

const JsAppLauncherErrorDesc* js_app_launcher_get_error_desc(const JsAppLauncher* instance);

JsAppLauncherError js_app_launcher_translate_from_js_runner_error(JsRunnerError js_runner_error);
