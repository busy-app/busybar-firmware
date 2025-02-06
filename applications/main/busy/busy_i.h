#pragma once

#include <furi.h>

#include <gui_lvgl/gui_lvgl.h>

#define TAG "Busy"

typedef enum {
    BusyAppSceneIdStart,
    BusyAppSceneIdBusy,
    BusyAppSceneIdMax,
} BusyAppSceneId;

typedef void (*BusyAppSceneOnEnter)(void* context);
typedef void (*BusyAppSceneOnExit)(void* context);
typedef void (*BusyAppSceneOnEvent)(uint32_t event, void* context);

typedef struct {
    BusyAppSceneOnEnter on_enter;
    BusyAppSceneOnExit on_exit;
    BusyAppSceneOnEvent on_event;
} BusyAppScene;

typedef void BusyAppSceneData;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    GuiLvgl* gui;
    BusyAppSceneData* scene_data[BusyAppSceneIdMax];
    const BusyAppScene* current_scene;
} BusyApp;

void busy_switch_to_scene(BusyApp* instance, BusyAppSceneId scene_id);
void busy_send_custom_event(BusyApp* instance, uint32_t event);
