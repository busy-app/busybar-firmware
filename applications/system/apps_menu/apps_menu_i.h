#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/scene_manager.h>

#include <storage/storage.h>

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    Gui* gui;

    SceneManager* scene_manager;
    Widget* front_scene_window;

    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;
} AppsMenu;

typedef enum {
    AppsMenuCustomEventLaunchMain,

    AppsMenuCustomEventMAX = 0xFFFFFFFF, // forces enum size, don't use
} AppsMenuCustomEvent;

static_assert(sizeof(AppsMenuCustomEvent) == sizeof(uint32_t));

#define APPS_MENU_ASSETS_PATH(path) EXT_PATH("apps_assets/apps_menu") "/" path
#define APPS_MENU_IMG(name)         APPS_MENU_ASSETS_PATH("images") "/" name ".bin"

#define APPS_MENU_NAV_BAR_HEIGHT 20

void apps_menu_send_custom_event(AppsMenu* app, AppsMenuCustomEvent event);
