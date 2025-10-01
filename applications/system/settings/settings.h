#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <audio/audio.h>
#include <back_display/back_display.h>
#include <front_display/front_display.h>
#include <matter/matter.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#include "helpers/wifi_poller.h"

typedef enum {
    SettingsCustomEventAboutToExit,

    SettingsCustomEventMatterCommStart,
    SettingsCustomEventMatterCommComplete,
    SettingsCustomEventMatterCommFail,

    SettingsCustomEventRequiredWifiNotAvailable,

    SettingsCustomEventSceneEventsStart,
} SettingsCustomEvent;

typedef struct SettingsApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;

    Gui* gui;
    Audio* audio;
    FrontDisplaySrv* front_display;
    BackDisplaySrv* back_display;

    MatterSrv* matter;
    FuriPubSubSubscription* matter_subscription;
    WifiPoller* wifi;

    Widget* front_scene_window;

    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;
} SettingsApp;

void settings_send_custom_event(SettingsApp* instance, uint32_t event);

void settings_push_location(SettingsApp* instance, const char* location_name);

void settings_pop_location(SettingsApp* instance);

bool settings_check_wifi_connectivity(SettingsApp* instance);
