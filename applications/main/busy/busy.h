#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <audio/audio.h>
#include <status_lights/status_lights.h>
#include <matter/matter.h>

#include "busy_timer.h"
#include "busy_settings.h"

#include "time_macros.h"
#include "storage_macros.h"

#include "helpers/run_later.h"
#include "scenes/busy_scenes.h"

#include "widgets/timer_bar.h"
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
    BusyCustomEventAboutToExit
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
    BusyTimerBarTypeWork,
    BusyTimerBarTypeRest,
    BusyTimerBarTypeMax,
} BusyTimerBarType;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    BusyTimer* busy_timer;
    StatusLights* status_lights;
    Audio* audio;
    Gui* gui;
    MatterSrv* matter;
    // Containers & application windows
    Widget* front_window;
    FlexLayout* back_container;
    Widget* back_window;
    // Persistent widgets
    TransitionOverlay* transition_overlay;
    TimerCard* timer_card;
    NavBar* nav_bar;
    // Application settings
    BusySettings settings;
} BusyApp;

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event);

void busy_prepare_transition(BusyApp* instance, BusyTransitionType type);

void busy_start_transition(BusyApp* instance);

void busy_set_status_lights(BusyApp* instance, BusyStatusLightsType type);

void busy_set_matter(BusyApp* instance, bool switch_state);

void busy_push_location(BusyApp* instance, const char* location_name);

void busy_pop_location(BusyApp* instance);
