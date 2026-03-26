/**
 * @brief Time settings app
 */

#pragma once

#include <furi.h>

#include <desktop/desktop.h>
#include <gui/gui.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <time/time.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include "scenes/time_scenes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define THIS_SETTINGS_APP "time_settings"
#define ASSETS_PATH(path) EXT_PATH("apps_assets/" THIS_SETTINGS_APP) "/" path
#define IMG_PATH(path)    ASSETS_PATH("images") "/" path
#define ANIM_PATH(path)   ASSETS_PATH("animations") "/" path

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;

    Desktop* desktop;
    Gui* gui;
    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;

    Widget* front_scene_window;

    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;

    Time* time;

} TimeSettingsApp;

void time_settings_send_custom_event(TimeSettingsApp* instance, uint32_t event);

#ifdef __cplusplus
}
#endif
