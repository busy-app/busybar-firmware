#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <audio/audio.h>
#include <status_lights/status_lights.h>

#include <busy/time_macros.h>
#include "storage_macros.h"

#include <busy/helpers/run_later.h>
#include "scenes/custom_scenes.h"

#include <busy/widgets/progress_bar.h>
#include <busy/widgets/timer_card.h>
#include <busy/widgets/timer_indicator.h>
#include <busy/widgets/transition_overlay.h>

#define TAG "Custom"

#define TOTAL_TIME_LOW_THR_MN (15)

typedef enum {
    CustomCustomEventTimerTick = 100,
    // CustomCustomEventTimerModeChanged,
    // CustomCustomEventTimerStateChanged,
    // CustomCustomEventTimerIntervalEnded,
    // CustomCustomEventTimerSequenceEnded,
    // CustomCustomEventTimerToggle,
    // CustomCustomEventTimerSkip,
    // CustomCustomEventTimeIncrement,
    // CustomCustomEventTimeDecrement,
    // CustomCustomEventStartPressed,
    // CustomCustomEventStartReleased,
    // CustomCustomEventStartShortPressed,
} CustomCustomEvent;

typedef enum {
    CustomTransitionTypeDefault,
    // CustomTransitionTypeAutomatic,
    // CustomTransitionTypeSkip,
    CustomTransitionTypeSelect,
    // CustomTransitionTypeWork,
    // CustomTransitionTypeRest,
    // CustomTransitionTypeWorkDone,
    // CustomTransitionTypeRestDone,
    CustomTransitionTypeMax,
} CustomTransitionType;

typedef enum {
    CustomStatusLightsTypeOff,
    CustomStatusLightsTypeWork,
    // CustomStatusLightsTypeRest,
    CustomStatusLightsTypeMax,
} CustomStatusLightsType;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;
    StatusLights* status_lights;
    Audio* audio;
    Gui* gui;
    // Containers & application windows
    Widget* front_window;
    FlexLayout* back_container;
    Widget* back_window;
    // Persistent widgets
    TransitionOverlay* transition_overlay;
    TimerCard* timer_card;
    NavBar* nav_bar;
} CustomApp;

void custom_send_custom_event(CustomApp* instance, uint32_t custom_event);

void custom_prepare_transition(CustomApp* instance, CustomTransitionType type);

void custom_start_transition(CustomApp* instance);

void custom_set_status_lights(CustomApp* instance, CustomStatusLightsType type);

void custom_push_location(CustomApp* instance, const char* location_name);

void custom_pop_location(CustomApp* instance);
