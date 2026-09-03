#pragma once

#include "scenes/scenes.h"
#include "settings/settings.h"

#include <gui/gui.h>
#include <time/time.h>
#include <updater/updater.h>
#include <storage/storage.h>

#include <gui/scene_manager.h>
#include <gui/modules/nav_bar.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/transition_overlay.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THIS_APP_NAME "clock"

#define THIS_ASSETS_PATH(path) EXT_PATH("apps_assets/" THIS_APP_NAME) "/" path
#define THIS_IMG_PATH(path)    THIS_ASSETS_PATH("images") "/" path
#define THIS_ANIM_PATH(path)   THIS_ASSETS_PATH("animations") "/" path

typedef enum {
    ClockEventTimerUpdate,

    ClockEventSceneEventsStart,
} ClockEvent;

typedef struct {
    bool do_skip_menu;
} ClockArguments;

typedef struct {
    ClockArguments arguments;

    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* timer;
    SceneManager* scene_manager;

    Gui* gui;
    Time* time;
    Updater* updater;

    ClockSettings settings;

    /* Front layout */
    TransitionOverlay* front_transition_overlay;
    Widget* front_scene_window;

    /* Back layout */
    FlexLayout* back_container;
    NavBar* back_nav_bar;
    Widget* back_scene_window;
} Clock;

void clock_internal_fire_event(Clock* instance, uint32_t event);

#ifdef __cplusplus
}
#endif
