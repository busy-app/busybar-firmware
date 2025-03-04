#pragma once

#include <furi.h>

#include <audio/audio.h>
#include <gui/gui.h>

#define TAG "Busy"

#define M_TO_S(m)     (m * 60)
#define H_TO_M(h)     (h * 60)
#define H_TO_S(h)     (M_TO_S(H_TO_M(h)))
#define HM_TO_M(h, m) (H_TO_M(h) + m)
#define HM_TO_S(h, m) (M_TO_S(H_TO_M(h, m)))

#define S_TO_M(s) (s / 60)
#define S_TO_R(s) (s % 60)
#define S_TO_H(h) (S_TO_M(s) / 60)

#define TOTAL_TIME_LOW_THR_MN (15)

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdTimer,
    BusyAppSceneIdStatic,
    BusyAppSceneIdQuit,
    BusyAppSceneIdNext,
    BusyAppSceneIdRestart,
    BusyAppSceneIdSetup,
    BusyAppSceneIdMax,
} BusyAppSceneId;

typedef enum {
    BusyTimerStateIdle,
    BusyTimerStateWork,
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
    BusyCustomEventIntervalEnd,
    BusyCustomEventSessionEnd,
    BusyCustomEventNext,
    BusyCustomEventBack,
    BusyCustomEventStartSingle,
    BusyCustomEventStartDouble,
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
    size_t data_size;
} BusyAppScene;

typedef void BusyAppSceneData;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    FuriEventLoopTimer* busy_timer;
    Gui* gui;
    Audio* audio;
    lv_obj_t* back_label;
    FuriPubSub* input_events;
    FuriPubSubSubscription* input_subscription;
    BusyAppSceneData* scene_data[BusyAppSceneIdMax];
    BusyAppSceneId current_scene_id;
    BusyTimerState state;
    uint32_t total_time_mn;
    uint32_t total_time_left_s;
    uint32_t work_time_mn;
    uint32_t short_rest_time_mn;
    uint32_t long_rest_time_mn;
    uint32_t intervals_total;
    uint32_t intervals_done;
    uint32_t interval_time_s;
    uint32_t interval_time_left_s;
    bool enable_intervals;
    bool enable_autostart_work;
    bool enable_autostart_rest;
    bool enable_autorestart_session;
    bool enable_sound;
    bool enable_speed;
} BusyApp;

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id);
void busy_send_custom_event(BusyApp* instance, uint32_t value);
void* busy_get_current_scene_data(BusyApp* instance);

void busy_timer_start(BusyApp* instance);
void busy_timer_stop(BusyApp* instance);
void busy_timer_next_state(BusyApp* instance, bool skip_event);
void busy_timer_pause(BusyApp* instance);
void busy_timer_resume(BusyApp* instance);
void busy_timer_toggle(BusyApp* instance);
bool busy_timer_is_running(BusyApp* instance);
