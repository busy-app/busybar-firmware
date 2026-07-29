#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include <js_app/js_app.h>

#define TAG "JsAppLauncher"

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
} JsAppLauncher;

typedef enum {
    JsAppLauncherCustomEventIndexMax = 0x7F,
} JsAppLauncherCustomEvent;

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event);
