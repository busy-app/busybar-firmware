#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <storage/storage.h>

#include "busy_timer.h"
#include "scenes/busy_scenes.h"
#include "compiled_assets/compiled_assets.h"

#define TAG "Busy"

#define TOTAL_TIME_LOW_THR_MN (15)

#define BUSY_ANIM_PATH(path) (APP_ASSETS_PATH("animations") "/" path)

typedef enum {
    BusyCustomEventUpdate = 100,
    BusyCustomEventIntervalEnd,
    BusyCustomEventSessionEnd,
    BusyCustomEventNext,
    BusyCustomEventBack,
    BusyCustomEventStartSingle,
    BusyCustomEventStartDouble,
} BusyCustomEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    BusyTimer* busy_timer;
    Gui* gui;
    Widget* front_window;
    Widget* back_window;
} BusyApp;

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event);
