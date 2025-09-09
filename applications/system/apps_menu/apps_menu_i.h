#pragma once

#include <furi.h>

#include <desktop/desktop.h>
#include <gui/gui.h>

#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/scene_manager.h>

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
} AppsMenu;

typedef enum {
    AppsMenuCustomEventLaunchMain,
    AppsMenuCustomEventAboutToExit,

    AppsMenuCustomEventMAX = 0xFFFFFFFF, // forces enum size, don't use
} AppsMenuCustomEvent;

static_assert(sizeof(AppsMenuCustomEvent) == sizeof(uint32_t));

void apps_menu_send_custom_event(AppsMenu* app, AppsMenuCustomEvent event);
