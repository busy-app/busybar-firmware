#pragma once

#include <furi.h>
#include <api_lock.h>

#include <gui/gui.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/mirror_card.h>
#include <gui/modules/transition_overlay.h>
#include <audio/audio.h>
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
    BusyStatus* status;
    FuriApiLock lock;
} BusyApiMessage;

struct BusyApp {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    FuriMessageQueue* api_queue;
    SceneManager* scene_manager;
    BusyTimer* busy_timer;
    FrontDisplaySrv* front_display;
    Audio* audio;
    Gui* gui;
    Updater* updater;
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

/**
 * @brief Adjust the loader priority to reflect the busy-timer activity state.
 *
 * Inactive (false): sets LOADER_PASSTHROUGH_PRIORITY so that HTTP canvas
 *   draw requests at DEFAULT priority (10) are accepted (canvas uses a
 *   strict-less-than comparison for empty-canvas draws).
 * Active (true): sets LOADER_BLOCKING_PRIORITY (= LOADER_MAX_PRIORITY + 1)
 *   so that every valid HTTP draw request (max API priority = 100) is
 *   unconditionally rejected while the timer is running.
 *
 * Called from:
 *   busy_scene_start_on_enter  → false (idle/start)
 *   busy_scene_timer_on_exit   → false (returning to start)
 *   busy_scene_timer_update_priority → mirrors timer running state
 *
 * @param[in] instance busy app instance
 * @param[in] active   true while the timer is actively running
 */
void busy_set_priority(BusyApp* instance, bool active);

void busy_set_front_display_blanking(BusyApp* instance, bool is_blanked);

void busy_push_location(BusyApp* instance, const char* location_name);

void busy_pop_location(BusyApp* instance);

bool busy_return_to_start_scene(BusyApp* instance);

void busy_exit(BusyApp* instance);

void busy_set_app_config(BusyApp* instance, const BusyAppConfig* app_config);

void busy_get_timer_preset(BusyApp* instance, BusyTimerPreset* timer_preset);

void busy_set_timer_preset(BusyApp* instance, BusyTimerPreset* timer_preset);

const BusyAppGlobalPreset* busy_get_global_preset(const BusyApp* instance);

BusyTimerProfileId busy_get_profile_id(const BusyApp* instance);

void busy_api_unlock_message(BusyApiMessage* api_message, BusyStatus status);

void busy_api_abort_pending_messages(BusyApp* instance);
