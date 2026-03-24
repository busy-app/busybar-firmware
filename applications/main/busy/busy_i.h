#pragma once

#include <furi.h>
#include <api_lock.h>

#include <gui/gui.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/mirror_card.h>
#include <gui/modules/transition_overlay.h>
#include <audio/audio.h>
#include <status_lights/status_lights.h>
#include <matter/matter.h>
#include <loader/loader.h>
#include <front_display/front_display.h>
#include <busy_timer/busy_timer.h>
#include <applications/system/updater/updater.h>

#include "busy.h"
#include "busy_theme.h"
#include "busy_presets.h"

#include "storage_macros.h"

#include "helpers/run_later.h"
#include "scenes/busy_scenes.h"

#include "widgets/timer_label.h"
#include "widgets/timer_indicator.h"

#define TAG "Busy"

#define TOTAL_TIME_LOW_THR_MN (15)

typedef enum {
    BusyAppRunModeNormal,
    BusyAppRunModeTimer,
    BusyAppRunModeMax,
} BusyAppRunMode;

typedef enum {
    BusyCustomEventIndexMax = 0x80,
    BusyCustomEventTimerTick,
    BusyCustomEventTimerModeChanged,
    BusyCustomEventTimerStateChanged,
    BusyCustomEventTimerIntervalEnded,
    BusyCustomEventTimerPaused,
    BusyCustomEventTimerSkip,
    BusyCustomEventTimeIncrement,
    BusyCustomEventTimeDecrement,
    BusyCustomEventStartPressed,
    BusyCustomEventStartReleased,
    BusyCustomEventStartShortPressed,
    BusyCustomEventOkShortPressed,
    BusyCustomEventReturnToStart,
    BusyCustomEventAnimationCompleted,
    BusyCustomEventAppConfigChanged,
    BusyCustomEventMax,
} BusyCustomEvent;

typedef enum {
    BusyApiMessageTypeSetConfig,
    BusyApiMessageTypeShowTimer,
    BusyApiMessageTypeRequestExit,
    BusyApiMessageTypeMax,
} BusyApiMessageType;

typedef struct {
    const BusyAppConfig* config;
} BusyApiMessageSetConfig;

typedef union {
    BusyApiMessageSetConfig set_config;
} BusyApiMessageData;

typedef struct {
    BusyApiMessageType type;
    BusyApiMessageData data;
    FuriApiLock lock;
} BusyApiMessage;

struct BusyApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    FuriMessageQueue* api_queue;
    SceneManager* scene_manager;
    BusyTimer* busy_timer;
    StatusLights* status_lights;
    FrontDisplaySrv* front_display;
    Audio* audio;
    Gui* gui;
    Updater* updater;
    MatterSrv* matter;
    Loader* loader;
    // Containers & application windows
    Widget* front_window;
    FlexLayout* back_container;
    Widget* back_window;
    // Persistent widgets
    TransitionOverlay* transition_overlay;
    MirrorCard* timer_card;
    NavBar* nav_bar;
    // Misc state
    BusyTheme* theme;
    BusyAppRunMode run_mode;
    BusyAppConfig config;
    BusyAppPresetId preset_id;
    bool show_timer_requested;
};

void busy_send_custom_event(BusyApp* instance, uint32_t custom_event);

void busy_prepare_transition(BusyApp* instance, BusyTransitionType type);

void busy_start_transition(BusyApp* instance);

void busy_set_status_lights(BusyApp* instance, BusyStatusLightsType type);

void busy_set_matter(BusyApp* instance, bool switch_state);

void busy_set_priority(BusyApp* instance, bool active);

void busy_set_front_display_blanking(BusyApp* instance, bool is_blanked);

void busy_push_location(BusyApp* instance, const char* location_name);

void busy_pop_location(BusyApp* instance);

bool busy_return_to_start_scene(BusyApp* instance);

void busy_exit(BusyApp* instance);

void busy_load_app_config(BusyApp* instance);

void busy_apply_app_config(BusyApp* instance);

void busy_get_timer_preset(BusyApp* instance, BusyTimerPreset* timer_preset);

void busy_set_timer_preset(BusyApp* instance, BusyTimerPreset* timer_preset);

const BusyAppGlobalPreset* busy_get_global_preset(const BusyApp* instance);

BusyTimerProfileId busy_get_profile_id(const BusyApp* instance);
