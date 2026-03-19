#pragma once

#include "scenes/scenes.h"

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <storage/storage.h>
#include <updater/updater.h>

#define TAG "UpdateUi"

#define THIS_ASSETS_PATH(path) EXT_PATH("apps_assets/update_ui") "/" path
#define THIS_IMG_PATH(path)    THIS_ASSETS_PATH("images") "/" path

typedef enum {
    UpdateUiEventSceneEventsStart,
} UpdateUiEvent;

typedef struct {
    FuriString* front_text;
    FuriString* back_primary_text;
    FuriString* back_detail_text;
} UpdateUiFailurePreset;

typedef struct {
    Gui* gui;
    Updater* updater;

    FuriEventLoop* event_loop;

    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;

    SceneManager* scene_manager;

    UpdateUiFailurePreset failure_preset;

    Widget* front_scene_window;
    Widget* back_scene_window;
} UpdateUi;

void update_ui_internal_fire_event(UpdateUi* instance, uint32_t event);
