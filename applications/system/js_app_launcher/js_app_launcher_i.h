#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>

#include <desktop/desktop.h>

#define TAG "JsAppLauncher"

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    Gui* gui;
    Desktop* desktop;
    const char* app_id;
} JsAppLauncher;

typedef enum {
    JsAppLauncherCustomEventIndexMax = 0x7F,
} JsAppLauncherCustomEvent;

void js_app_launcher_send_custom_event(JsAppLauncher* instance, uint32_t event);
