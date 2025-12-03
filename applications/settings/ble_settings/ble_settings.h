/**
 * @brief Brightness settings app
 */

#pragma once

#include <furi.h>

#include <ble/ble.h>
#include <desktop/desktop.h>
#include <gui/gui.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <status_lights/status_lights.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include "scenes/ble_scenes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AppEventAboutToExit,
    AppEventSceneEventsStart,
} AppEvent;

#define THIS_SETTINGS_APP "ble_settings"
#define ASSETS_PATH(path) EXT_PATH("apps_assets/" THIS_SETTINGS_APP) "/" path
#define IMG_PATH(path)    ASSETS_PATH("images") "/" path

typedef struct {
    Ble* ble;
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;

    StatusLights* status_lights;
    Desktop* desktop;
    Gui* gui;
    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;

    Widget* front_scene_window;
    Widget* back_scene_window;

    FlexLayout* back_container;

    NavBar* back_nav_bar;
} BleSettings;

void ble_settings_send_custom_event(BleSettings* instance, uint32_t event);

#ifdef __cplusplus
}
#endif
