#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include <desktop/desktop.h>

typedef enum {
    SettingsCustomEventAboutToExit,

    SettingsCustomEventSceneEventsStart,
} SettingsCustomEvent;

typedef struct SettingsApp {
    const char* launching_subapp;
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
} SettingsApp;

void settings_send_custom_event(SettingsApp* instance, uint32_t event);
