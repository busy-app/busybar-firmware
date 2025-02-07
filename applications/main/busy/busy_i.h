#pragma once

#include <furi.h>

#include <gui_lvgl/gui_lvgl.h>

#define TAG "Busy"

#define BUSY_INTERVAL_INFINITE (UINT32_MAX)

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdTimer,
    BusyAppSceneIdStatic,
    BusyAppSceneIdMax,
} BusyAppSceneId;

typedef enum {
    BusyTimerStateIdle,
    BusyTimerStateBusy,
    BusyTimerStateRest,
    BusyTimerStateLongRest,
    BusyTimerStateMax,
} BusyTimerState;

typedef enum {
    BusyEventTypeStart,
    BusyEventTypeBack,
    BusyEventTypeOk,
    BusyEventTypeCustom,
} BusyEventType;

typedef enum {
    BusyCustomEventUpdate = 100,
} BusyCustomEvent;

typedef struct {
    BusyEventType type;
    uint32_t custom_value;
} BusyEvent;

typedef void (*BusyAppSceneOnEnter)(void* context);
typedef void (*BusyAppSceneOnExit)(void* context);
typedef void (*BusyAppSceneOnEvent)(const BusyEvent* event, void* context);

typedef struct {
    BusyAppSceneOnEnter on_enter;
    BusyAppSceneOnExit on_exit;
    BusyAppSceneOnEvent on_event;
} BusyAppScene;

typedef void BusyAppSceneData;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* busy_timer;
    GuiLvgl* gui;
    FuriPubSubSubscription* input_events;
    BusyAppSceneData* scene_data[BusyAppSceneIdMax];
    const BusyAppScene* current_scene;
    uint32_t busy_interval_s;
    uint32_t rest_interval_s;
    uint32_t long_rest_interval_s;
    uint32_t cycles_count;
    uint32_t time_total;
    uint32_t time_left;
    uint32_t cycles_left;
    BusyTimerState state;
} BusyApp;

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id);
void busy_send_custom_event(BusyApp* instance, uint32_t value);

void busy_timer_start(BusyApp* instance);
void busy_timer_stop(BusyApp* instance);
void busy_timer_next_state(BusyApp* instance);
void busy_timer_pause_toggle(BusyApp* instance);
