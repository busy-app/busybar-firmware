#pragma once

#include <furi.h>

#include <gui/gui.h>
#include <gui/modules/label.h>

#include <storage/storage.h>

#include "image_animation.h"

typedef enum {
    AnimationPlayerAppEventExit,
} AnimationPlayerAppEvent;

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* event_queue;
    Gui* gui;

    ImageAnimation* image_animation;
    Label* label;
} AnimationPlayerApp;
