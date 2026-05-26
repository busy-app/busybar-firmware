#pragma once

#include "scenes/power_on_scenes.h"

#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <power/power_service/power.h>
#include <storage/storage.h>
// #include <gui/modules/label.h>
// #include <gui/modules/anim_player.h>

#define TAG "PowerOn"

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* input_queue;
    FuriMessageQueue* event_queue;
    SceneManager* scene_manager;

    Gui* gui;
    Power* power;
    Storage* storage;

    Widget* front_root;
    Widget* back_root;
} PowerOnApp;

typedef enum {
    PowerOnAppEventStarted,
    PowerOnAppEventShutdown,
    PowerOnAppEventUserInteracted,

    PowerOnAppEventMAX,
} PowerOnAppEvent;

bool power_on_is_done_flag_present(PowerOnApp* instance);

void power_on_done_flag_create(PowerOnApp* instance);

void power_on_send_custom_event(PowerOnApp* instance, uint32_t event);

/**
 * Sends `PowerOnAppEventUserInteracted` on any `Short` input.
 */
bool power_on_handle_generic_input(PowerOnApp* instance, const InputEvent* event);
