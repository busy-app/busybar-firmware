#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <storage/storage.h>

#include <assets/assets_images.h>

#include "busy_timer.h"
#include "time_macros.h"

#include "helpers/run_later.h"
#include "scenes/busy_scenes.h"

#define TAG "Busy"

#define TOTAL_TIME_LOW_THR_MN (15)

#define BUSY_ASSETS_PATH(path) EXT_PATH("apps_assets/busy") "/" path
#define BUSY_ANIM_PATH(path)   BUSY_ASSETS_PATH("animations") "/" path
#define BUSY_IMG_PATH(path)    BUSY_ASSETS_PATH("images") "/" path

typedef enum {
    BusyCustomEventTimerTick = 100,
    BusyCustomEventTimerStateChanged,
    BusyCustomEventTimerIntervalEnded,
    BusyCustomEventTimerSequenceEnded,
    BusyCustomEventTimerToggle,
    BusyCustomEventTimerSkip,
    BusyCustomEventTimeIncrement,
    BusyCustomEventTimeDecrement,
    BusyCustomEventStartPressed,
    BusyCustomEventStartReleased,
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
