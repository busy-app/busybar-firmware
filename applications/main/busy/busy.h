#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <audio/audio.h>
#include <status_lights/status_lights.h>

#include "busy_timer.h"
#include "busy_settings.h"

#include "time_macros.h"
#include "storage_macros.h"

#include "helpers/run_later.h"
#include "scenes/busy_scenes.h"

#include "widgets/nav_stack.h"
#include "widgets/progress_bar.h"
#include "widgets/timer_card.h"
#include "widgets/timer_indicator.h"
#include "widgets/transition_overlay.h"

#define TAG "Busy"

#define TOTAL_TIME_LOW_THR_MN (15)

typedef enum {
    BusyCustomEventTimerTick = 100,
    BusyCustomEventTimerModeChanged,
    BusyCustomEventTimerStateChanged,
    BusyCustomEventTimerIntervalEnded,
    BusyCustomEventTimerSequenceEnded,
    BusyCustomEventTimerToggle,
    BusyCustomEventTimerSkip,
    BusyCustomEventTimeIncrement,
    BusyCustomEventTimeDecrement,
    BusyCustomEventStartPressed,
    BusyCustomEventStartReleased,
    BusyCustomEventStartShortPressed,
} BusyCustomEvent;

typedef enum {
    BusyTransitionTypeDefault,
    BusyTransitionTypeAutomatic,
    BusyTransitionTypeSkip,
    BusyTransitionTypeSelect,
    BusyTransitionTypeWork,
    BusyTransitionTypeRest,
    BusyTransitionTypeWorkDone,
    BusyTransitionTypeRestDone,
    BusyTransitionTypeMax,
} BusyTransitionType;

typedef enum {
    BusyStatusLightsTypeOff,
    BusyStatusLightsTypeWork,
    BusyStatusLightsTypeRest,
    BusyStatusLightsTypeMax,
} BusyStatusLightsType;

typedef enum {
    BusyProgressBarTypeWork,
    BusyProgressBarTypeRest,
    BusyProgressBarTypeMax,
} BusyProgressBarType;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    BusyTimer* busy_timer;
    StatusLights* status_lights;
    Audio* audio;
    Gui* gui;
    // Application windows
    Widget* front_window;
    Widget* back_window;
    // Persistent widgets
    TransitionOverlay* transition_overlay;
    TimerCard* timer_card;
    NavStack* nav_stack;
    // Application settings
    BusySettings settings;
} BusyApp;

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event);

void busy_prepare_transition(BusyApp* instance, BusyTransitionType type);

void busy_start_transition(BusyApp* instance);

void busy_set_status_lights(BusyApp* instance, BusyStatusLightsType type);

void busy_push_location(BusyApp* instance, const char* location_name);

void busy_pop_location(BusyApp* instance);
