/**
 * @brief Account settings app
 */

#pragma once

#include <furi.h>

#include <desktop/desktop.h>
#include <gui/gui.h>
#include <front_display/front_display.h>
#include <back_display/back_display.h>
#include <status_lights/status_lights.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>

#include "scenes/account_scenes.h"
#include "models/account_model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AppEventAboutToExit,
    AppEventAccountStateChange,
    AppEventAccountLinkPin,
    AppEventAccountLinkPinTimeout,
    AppEventAccountLinkDone,
    AppEventAccountUnlinked,
    AppEventSceneEventsStart,
} AppEvent;

#define THIS_SETTINGS_APP "account_settings"
#define ASSETS_PATH(path) EXT_PATH("apps_assets/" THIS_SETTINGS_APP) "/" path
#define IMG_PATH(path)    ASSETS_PATH("images") "/" path

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
    Widget* back_scene_window;

    FlexLayout* back_container;

    NavBar* back_nav_bar;

    AccountModel* model;

    char link_pin[ACCOUNT_MODEL_LINK_PIN_LEN + 1];
} AccountSettings;

void account_settings_send_custom_event(AccountSettings* instance, uint32_t event);

#ifdef __cplusplus
}
#endif
