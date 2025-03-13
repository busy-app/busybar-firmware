#pragma once

#include <furi.h>
#include <gui_lvgl/gui_lvgl.h>
#include <led_display/led_display.h>
#include <storage/storage.h>

#include "image_animation.h"

#define ANIMATION_PLAYER_CASH_SIZE    (10)
#define ANIMATION_PLAYER_PRELOAD_SIZE (ANIMATION_PLAYER_CASH_SIZE / 5)

typedef enum {
    AnimationPlayerAppEventExit,
} AnimationPlayerAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    GuiLvgl* gui;

    ImageAnimation* image_animation;
    lv_obj_t* label;
} AnimationPlayerApp;
