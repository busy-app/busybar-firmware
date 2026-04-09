#pragma once

#include "scenes/scenes.h"

#include <gui/gui.h>
#include <desktop/desktop.h>
#include <updater/updater.h>
#include <storage/storage.h>
#include <power/power_service/power.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "FirmwareSettings"

#define THIS_APP_NAME "firmware_settings"

#define THIS_ASSETS_PATH(path) EXT_PATH("apps_assets/" THIS_APP_NAME) "/" path
#define THIS_IMG_PATH(path)    THIS_ASSETS_PATH("images") "/" path

typedef enum {
    FirmwareSettingsEventSceneEventsStart,
} FirmwareSettingsEvent;

typedef struct {
    const char* front_image_path;
    FuriString* front_text;

    const char* back_image_path;
    FuriString* back_primary_text;
    FuriString* back_detail_text;
} FirmwareSettingsCheckResultScenePreset;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;

    Gui* gui;
    Desktop* desktop;
    Updater* updater;
    Power* power;

    UpdateCheckInfo update_info;
    FirmwareSettingsCheckResultScenePreset check_result_preset;

    /* front layout */
    Widget* front_scene_window;

    /* back layout */
    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;
} FirmwareSettings;

void firmware_settings_internal_fire_event(FirmwareSettings* instance, uint32_t event);

#ifdef __cplusplus
}
#endif
