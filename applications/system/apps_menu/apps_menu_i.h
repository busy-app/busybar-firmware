#pragma once

#include <furi.h>

#include <desktop/desktop.h>
#include <gui/gui.h>

#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/scene_manager.h>

#include "settings/settings.h"

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    Desktop* desktop;
    Gui* gui;

    SceneManager* scene_manager;
    Widget* front_scene_window;

    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;

    AppsMenuSettings settings;
} AppsMenu;

typedef enum {
    AppsMenuCustomEventLaunchMain,
    AppsMenuCustomEventAboutToExit,

    AppsMenuCustomEventSceneEventsStart,

    AppsMenuCustomEventMAX = 0xFFFFFFFF, // forces enum size, don't use
} AppsMenuCustomEvent;

static_assert(sizeof(AppsMenuCustomEvent) == sizeof(uint32_t));

void apps_menu_send_custom_event(AppsMenu* app, AppsMenuCustomEvent event);

bool apps_menu_start_application(const char* app_id, bool is_skip_menu);

void apps_menu_set_active_application(AppsMenuSettings* settings, const char* app_id);

// TODO: Remove this when JS apps support is fully functional
bool apps_menu_is_js_apps_enabled(void);
